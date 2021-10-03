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
#include <shared_mutex>
#include <utility>

#include "connection.hpp"
#include "script.hpp"
#include "script_queue.hpp"
#include "stats.hpp"

namespace ng = nghttp2::asio_http2;
using ng::header_map;
using ng::header_value;
using namespace std::chrono;

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
      conn(std::make_unique<connection>(h, p, secure_session)),
      mtx()
{
    if (!conn->wait_to_be_connected())
    {
        std::cerr << "Fatal error. Could not connect to: " << host << ":" << port << std::endl;
    }
}

std::string client_impl::get_uri(const std::string& uri_path)
{
    return "http://" + host + ":" + port + "/" + uri_path;
}

void client_impl::handle_timeout(const std::shared_ptr<race_control>& control,
                                 const std::string& msg_name)
{
    std::lock_guard<std::mutex> guard(control->mtx);
    if (control->answered)
    {
        return;
    }
    control->timed_out = true;
    stats->add_timeout(msg_name);
    queue->cancel_script();
}

void client_impl::handle_timeout_cancelled(const std::shared_ptr<race_control>& control,
                                           const std::string& msg_name)
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
                             std::shared_ptr<race_control> control, std::string msg_name)
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

request client_impl::get_next_request(const traffic::script& s)
{
    std::string body = s.get_next_body();
    header_value type = {"application/json", false};
    header_value length = {std::to_string(body.length()), false};
    header_map map = {{"content-type", type}, {"content-length", length}};

    return request{get_uri(s.get_next_url()), s.get_next_method(), body, map,
                   s.get_next_msg_name()};
}

void client_impl::open_new_connection()
{
    if (!mtx.try_lock())
    {
        return;
    }
    conn.reset();

    auto new_conn = std::make_unique<connection>(host, port, secure_session);
    if (new_conn->wait_to_be_connected())
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
    if (!script_opt.is_initialized())
    {
        return;
    }
    auto script = script_opt.get();
    request req = get_next_request(script);

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

    auto& session = conn->get_session();
    session.io_service().post([this, script, &session, req] {
        boost::system::error_code ec;
        auto init_time = std::make_shared<time_point<steady_clock>>(steady_clock::now());
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

        auto ctrl = std::make_shared<race_control>();
        auto timer = std::make_shared<boost::asio::steady_timer>(io_ctx);
        timer->expires_after(milliseconds(script.get_timeout_ms()));
        timer->async_wait(boost::bind(&client_impl::on_timeout, this,
                                      boost::asio::placeholders::error, ctrl, req.name));

        nghttp_req->on_response(
            [this, timer, init_time, script, ctrl, req](const ng::client::response& res) {
                auto elapsed_time =
                    duration_cast<microseconds>(steady_clock::now() - (*init_time)).count();

                std::lock_guard<std::mutex> guard(ctrl->mtx);
                if (ctrl->timed_out)
                {
                    return;
                }
                ctrl->answered = true;
                timer->cancel();

                auto answer = std::make_shared<std::string>();
                res.on_data([this, &res, script, answer, elapsed_time, req](const uint8_t* data,
                                                                            std::size_t len) {
                    if (len > 0)
                    {
                        std::string json(reinterpret_cast<const char*>(data), len);
                        *answer += json;
                    }
                    else
                    {
                        traffic::answer_type ans = std::make_pair(res.status_code(), *answer);
                        bool valid_answer = script.validate_answer(ans);
                        if (valid_answer)
                        {
                            stats->add_measurement(req.name, elapsed_time, res.status_code());
                            queue->enqueue_script(script, ans);
                        }
                        else
                        {
                            stats->add_error(req.name, res.status_code());
                            queue->cancel_script();
                        }
                    }
                });
            });

        nghttp_req->on_close([](uint32_t error_code) {});
    });
    mtx.unlock_shared();
}

}  // namespace http2_client
