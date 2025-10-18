#include "client_impl.hpp"

#include <nghttp2/asio_http2_client.h>
#include <syslog.h>

#include <atomic>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <utility>

#include "connection.hpp"
#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "opentelemetry/semconv/incubating/http_attributes.h"
#include "opentelemetry/semconv/url_attributes.h"
#include "script.hpp"
#include "script_queue.hpp"
#include "stats.hpp"
#include "tracer.hpp"

namespace ng = nghttp2::asio_http2;
using namespace std::chrono;
namespace ot_trace = opentelemetry::trace;
namespace ot_conv = opentelemetry::semconv;

namespace http2_client
{
client_impl::client_impl(std::shared_ptr<stats::stats_if> st, boost::asio::io_context& io_ctx,
                         std::unique_ptr<traffic::script_queue_if> q, const std::string& h,
                         const std::string& p, const bool secure_session)
    : stats(std::move(st)),
      io_ctx(io_ctx),
      queue(std::move(q)),
      host(h),
      port(p),
      secure_session(secure_session),
      conn(std::make_unique<connection>(h, p, secure_session))
{
    if (!conn->wait_to_be_connected())
    {
        std::cerr << "Fatal error. Could not connect to: " << host << ":" << port << std::endl;
    }
}

void client_impl::handle_timeout(const std::shared_ptr<race_control>& control,
                                 const std::string& msg_name) const
{
    std::scoped_lock guard(control->mtx);
    if (control->answered)
    {
        return;
    }
    control->timed_out = true;
    stats->add_timeout(msg_name);
    queue->cancel_script();
}
// TODO: Add timeout handling in spans
void client_impl::handle_timeout_cancelled(const std::shared_ptr<race_control>& control,
                                           const std::string& msg_name) const
{
    if (control->mtx.try_lock())
    {
        if (!control->answered)
        {
            control->timed_out = true;
            stats->add_error(msg_name, 469);
            queue->cancel_script();
        }
        control->mtx.unlock();
    }
}

void client_impl::on_timeout(const boost::system::error_code& e,
                             std::shared_ptr<race_control> control,
                             const std::string& msg_name) const
{
    if (e.value() == 0)
    {
        handle_timeout(control, msg_name);
    }
    else
    {
        handle_timeout_cancelled(control, msg_name);
    }
}

void client_impl::open_new_connection()
{
    if (!mtx.try_lock())
    {
        return;
    }
    conn.reset();

    if (auto new_conn = std::make_unique<connection>(host, port, secure_session);
        new_conn->wait_to_be_connected())
    {
        conn = std::move(new_conn);
    }
    else
    {
        new_conn.reset();
    }
    mtx.unlock();
}

void client_impl::send()
{
    auto script_opt = queue->get_next_script();
    if (!script_opt.has_value())
    {
        return;
    }
    const auto& script = *script_opt;
    request req = get_next_request(host, port, script);

    if (!is_connected())
    {
        stats->add_client_error(req.name, 466);
        queue->cancel_script();
        open_new_connection();
        return;
    }

    if (!mtx.try_lock_shared())
    {
        stats->add_client_error(req.name, 467);
        queue->cancel_script();
        return;
    }

    const auto& session = conn->get_session();
    session.io_service().post(
        [this, script, &session, req] () mutable
        {
            boost::system::error_code ec;
            auto init_time = std::make_shared<time_point<steady_clock>>(steady_clock::now());

            auto span = o11y::create_child_span(req.name, script.get_span());
            span->SetAttribute(ot_conv::url::kUrlFull, req.url);
            span->SetAttribute(ot_conv::http::kHttpRequestMethod, req.method);
            o11y::inject_trace_context(span, req.headers);

            auto nghttp_req = session.submit(ec, req.method, req.url, req.body, req.headers);
            if (!nghttp_req)
            {
                std::cerr << "Error submitting. Closing connection:" << ec.message() << std::endl;
                conn->close();
                stats->add_client_error(req.name, 468);
                queue->cancel_script();
                return;
            }

            stats->increase_sent(req.name);
            span->AddEvent("Request sent");

            auto ctrl = std::make_shared<race_control>();
            auto timer = std::make_shared<boost::asio::steady_timer>(io_ctx);
            timer->expires_after(milliseconds(script.get_timeout_ms()));
            timer->async_wait(boost::bind(&client_impl::on_timeout, this,
                                          boost::asio::placeholders::error, ctrl, req.name));

            nghttp_req->on_response(
                [this, timer, init_time, script, ctrl, req, span](const ng::client::response& res)
                {
                    auto elapsed_time =
                        duration_cast<microseconds>(steady_clock::now() - (*init_time)).count();

                    std::lock_guard guard(ctrl->mtx);
                    if (ctrl->timed_out)
                    {
                        return;
                    }
                    ctrl->answered = true;
                    timer->cancel();

                    span->AddEvent("Response received");
                    auto answer = std::make_shared<std::string>();
                    res.on_data(
                        [this, &res, script, answer, elapsed_time, req, span](const uint8_t* data,
                                                                              std::size_t len)
                        {
                            if (len > 0)
                            {
                                std::string json(reinterpret_cast<const char*>(data), len);
                                *answer += json;
                            }
                            else
                            {
                                span->AddEvent("Body received");
                                traffic::answer_type ans = {res.status_code(), *answer,
                                                            res.header()};
                                span->SetAttribute(ot_conv::http::kHttpResponseStatusCode,
                                                   res.status_code());

                                bool valid_answer = script.validate_answer(ans);
                                if (valid_answer)
                                {
                                    stats->add_measurement(req.name, elapsed_time,
                                                           res.status_code());
                                    span->SetStatus(ot_trace::StatusCode::kOk);
                                    span->End();
                                    queue->enqueue_script(script, ans);
                                }
                                else
                                {
                                    stats->add_error(req.name, res.status_code());
                                    span->SetStatus(opentelemetry::trace::StatusCode::kError);
                                    span->End();
                                    queue->cancel_script();
                                }
                            }
                        });
                });

            nghttp_req->on_close(
                []([[maybe_unused]] uint32_t error_code)
                {
                    // on_close is registered here for the sake of completion and
                    // because it helps debugging sometimes, but no implementation needed.
                });
        });
    mtx.unlock_shared();
}

}  // namespace http2_client
