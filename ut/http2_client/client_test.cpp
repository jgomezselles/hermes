#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nghttp2/asio_http2_server.h>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <thread>

#include "client_impl.hpp"
#include "script_builder.hpp"
#include "stats_if.hpp"
#include "script_queue_if.hpp"

using namespace std::chrono_literals;
namespace ba = boost::asio;
namespace ng = nghttp2::asio_http2;

using testing::_;
using testing::Return;

class stats_mock : public stats::stats_if
{
public:
    MOCK_METHOD1(increase_sent, void(const std::string &));
    MOCK_METHOD3(add_measurement, void(const std::string &, const int64_t, const int));
    MOCK_METHOD1(add_timeout, void(const std::string &));
    MOCK_METHOD2(add_error, void(const std::string &, const int));
    MOCK_METHOD2(add_client_error, void(const std::string &, const int));
};

class script_queue_mock : public traffic::script_queue_if
{
public:
    MOCK_METHOD0(get_next_script, boost::optional<traffic::script>());
    MOCK_METHOD2(enqueue_script, void(traffic::script, const traffic::answer_type&));
    MOCK_METHOD0(cancel_script, void());
    MOCK_CONST_METHOD0(has_pending_scripts, bool());
    MOCK_METHOD0(close_window, void());
    MOCK_METHOD0(is_window_closed, bool());
};

namespace http2_client
{
class client_test : public ::testing::Test
{
public:
    client_test()
        : server_host("localhost"),
          server_port("8080"),
          server_started(false),
          client_io_ctx(),
          client_io_ctx_guard(ba::make_work_guard(client_io_ctx))
    {
        client_worker = std::thread([this]() { client_io_ctx.run(); });
    };

    void start_server()
    {
        boost::system::error_code server_error_code;

        server = std::make_unique<ng::server::http2>();
        server->handle("/v1/test",
                       [](const ng::server::request &req, const ng::server::response &res) {
                           res.write_head(200);
                           res.end();
                       });

        if (server->listen_and_serve(server_error_code, server_host, server_port, true))
        {
            fprintf(stderr, "Error starting server in %s:%s", server_host.c_str(),
                    server_port.c_str());
        }

        server_started = true;
    }

    void stop_server()
    {
        for (auto &service : server->io_services())
        {
            service->stop();
        }
        server->stop();
        server->join();
        server.reset();
        server_started = false;
    }

    void SetUp() override
    {
        start_server();
    };

    void TearDown() override
    {
        if (server_started)
        {
            stop_server();
        }
        client_io_ctx_guard.reset();
        client_worker.join();
    };

protected:
    std::unique_ptr<nghttp2::asio_http2::server::http2> server;
    const std::string server_host;
    const std::string server_port;
    bool server_started;

    ba::io_context client_io_ctx;
    ba::executor_work_guard<ba::io_context::executor_type> client_io_ctx_guard;
    std::thread client_worker;
};

ACTION_P(SetFuture, prom)
{
    prom->set_value();
};

TEST_F(client_test, my_first_test)
{
    std::promise<void> prom;
    std::future<void> fut = prom.get_future();

    auto stats = std::make_shared<stats_mock>();

    EXPECT_CALL(*stats, increase_sent("test1")).Times(1);
    EXPECT_CALL(*stats, add_measurement("test1" ,_ ,200)).Times(1);

    auto queue = std::make_unique<script_queue_mock>();

    ut_helpers::script_builder script_builder;
    script_builder.dns(server_host).port(server_port);
    std::stringstream json_stream;
    json_stream << script_builder.build();

    boost::optional<traffic::script> script(json_stream);
    traffic::answer_type ans = std::make_pair(200, "");

    EXPECT_CALL(*queue, get_next_script()).Times(1).WillOnce(Return(script));
    EXPECT_CALL(*queue, enqueue_script(_, ans)).Times(1).WillOnce(SetFuture(&prom));

    auto client = client_impl(stats, client_io_ctx, std::move(queue), server_host, server_port);
    client.send();

    ASSERT_EQ(fut.wait_for(1s), std::future_status::ready);
}

}  // namespace http2_client
