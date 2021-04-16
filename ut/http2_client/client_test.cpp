#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nghttp2/asio_http2_server.h>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <thread>

#include "client_impl.hpp"
#include "message_builder.hpp"
#include "script_builder.hpp"
#include "script_queue_if.hpp"
#include "stats_if.hpp"

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
    MOCK_METHOD2(enqueue_script, void(traffic::script, const traffic::answer_type &));
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
                       [this](const ng::server::request &req, const ng::server::response &res) {
                           res.write_head(200);
                           res.end(response_body);
                       });

        server->handle("/v1/test_timeout",
                       [](const ng::server::request &req, const ng::server::response &res) {
                           std::this_thread::sleep_for(750ms);
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

    void SetUp() override { start_server(); };

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

    std::string response_body{"Example"};
};

ACTION_P(SetFuture, prom)
{
    prom->set_value();
};

TEST_F(client_test, HasFinishedTrueWhenQueueEmpty)
{
    auto stats = std::make_shared<stats_mock>();
    auto queue = std::make_unique<script_queue_mock>();
    EXPECT_CALL(*queue, has_pending_scripts()).Times(1).WillOnce(Return(false));

    auto client = client_impl(stats, client_io_ctx, std::move(queue), server_host, server_port);

    ASSERT_TRUE(client.is_connected());
    ASSERT_TRUE(client.has_finished());
}

TEST_F(client_test, HasFinishedFalseWhenQueueIsNotEmpty)
{
    auto stats = std::make_shared<stats_mock>();
    auto queue = std::make_unique<script_queue_mock>();
    EXPECT_CALL(*queue, has_pending_scripts()).Times(1).WillOnce(Return(true));

    auto client = client_impl(stats, client_io_ctx, std::move(queue), server_host, server_port);

    ASSERT_TRUE(client.is_connected());
    ASSERT_FALSE(client.has_finished());
}

TEST_F(client_test, CloseWindowNotifiesQueueToStopQueuing)
{
    auto stats = std::make_shared<stats_mock>();
    auto queue = std::make_unique<script_queue_mock>();
    EXPECT_CALL(*queue, close_window()).Times(1);

    auto client = client_impl(stats, client_io_ctx, std::move(queue), server_host, server_port);

    ASSERT_TRUE(client.is_connected());
    client.close_window();
}

TEST_F(client_test, ConnectToServer)
{
    auto stats = std::make_shared<stats_mock>();
    auto queue = std::make_unique<script_queue_mock>();
    auto client = client_impl(stats, client_io_ctx, std::move(queue), server_host, server_port);

    ASSERT_TRUE(client.is_connected());
}

TEST_F(client_test, ErrorConnectingToServer)
{
    auto stats = std::make_shared<stats_mock>();
    auto queue = std::make_unique<script_queue_mock>();
    auto client = client_impl(stats, client_io_ctx, std::move(queue), "wrong_host", server_port);

    ASSERT_FALSE(client.is_connected());
}

TEST_F(client_test, SendMessage)
{
    auto stats = std::make_shared<stats_mock>();
    EXPECT_CALL(*stats, increase_sent("test1")).Times(1);
    EXPECT_CALL(*stats, add_measurement("test1", _, 200)).Times(1);

    auto queue = std::make_unique<script_queue_mock>();

    ut_helpers::script_builder script_builder;
    script_builder.dns(server_host).port(server_port);
    std::stringstream json_stream;
    json_stream << script_builder.build();

    boost::optional<traffic::script> script(json_stream);
    traffic::answer_type ans = std::make_pair(200, response_body);
    std::promise<void> prom;
    std::future<void> fut = prom.get_future();
    EXPECT_CALL(*queue, get_next_script()).Times(1).WillOnce(Return(script));
    EXPECT_CALL(*queue, enqueue_script(_, ans)).Times(1).WillOnce(SetFuture(&prom));

    auto client = client_impl(stats, client_io_ctx, std::move(queue), server_host, server_port);

    ASSERT_TRUE(client.is_connected());

    client.send();

    ASSERT_EQ(fut.wait_for(1s), std::future_status::ready);
}

TEST_F(client_test, TimeoutInAnswer)
{
    auto stats = std::make_shared<stats_mock>();
    EXPECT_CALL(*stats, increase_sent("test1")).Times(1);
    EXPECT_CALL(*stats, add_timeout("test1")).Times(1);

    auto queue = std::make_unique<script_queue_mock>();

    ut_helpers::script_builder script_builder;
    script_builder.dns(server_host)
        .port(server_port)
        .timeout(500)
        .messages(std::vector<std::string>{
            ut_helpers::message_builder(1).url("v1/test_timeout").build()});

    std::stringstream json_stream;
    json_stream << script_builder.build();
    boost::optional<traffic::script> script(json_stream);

    std::promise<void> prom;
    std::future<void> fut = prom.get_future();
    EXPECT_CALL(*queue, get_next_script()).Times(1).WillOnce(Return(script));
    EXPECT_CALL(*queue, cancel_script()).Times(1).WillOnce(SetFuture(&prom));

    auto client = client_impl(stats, client_io_ctx, std::move(queue), server_host, server_port);

    ASSERT_TRUE(client.is_connected());

    client.send();

    ASSERT_EQ(fut.wait_for(1s), std::future_status::ready);
}

TEST_F(client_test, WrongCodeInAnswer)
{
    auto stats = std::make_shared<stats_mock>();
    EXPECT_CALL(*stats, increase_sent("test1")).Times(1);
    EXPECT_CALL(*stats, add_error("test1", 404)).Times(1);

    auto queue = std::make_unique<script_queue_mock>();

    ut_helpers::script_builder script_builder;
    script_builder.dns(server_host)
        .port(server_port)
        .messages(
            std::vector<std::string>{ut_helpers::message_builder(1).url("v1/wrong_url").build()});

    std::stringstream json_stream;
    json_stream << script_builder.build();
    boost::optional<traffic::script> script(json_stream);

    std::promise<void> prom;
    std::future<void> fut = prom.get_future();
    EXPECT_CALL(*queue, get_next_script()).Times(1).WillOnce(Return(script));
    EXPECT_CALL(*queue, cancel_script()).Times(1).WillOnce(SetFuture(&prom));

    auto client = client_impl(stats, client_io_ctx, std::move(queue), server_host, server_port);

    ASSERT_TRUE(client.is_connected());

    client.send();

    ASSERT_EQ(fut.wait_for(1s), std::future_status::ready);
}

TEST_F(client_test, ServerDisconnectionTriggersReconnectionInNextMessage)
{
    // This test tries to send a message 3 times.
    // 1) There's no connection, so error is added and script in queue cancelled.
    //    A connection is then triggered but unsuccessful.
    // 2) After server is respawned, the same happens, but this time the reconnection occurs.
    // 3) Message is sent successfully

    auto stats = std::make_shared<stats_mock>();
    EXPECT_CALL(*stats, add_client_error("test1", _)).Times(2);
    EXPECT_CALL(*stats, increase_sent("test1")).Times(1);
    EXPECT_CALL(*stats, add_measurement("test1", _, 200)).Times(1);

    auto queue = std::make_unique<script_queue_mock>();

    ut_helpers::script_builder script_builder;
    script_builder.dns(server_host).port(server_port);
    std::stringstream json_stream;
    json_stream << script_builder.build();

    boost::optional<traffic::script> script(json_stream);
    traffic::answer_type ans = std::make_pair(200, response_body);
    std::promise<void> prom1, prom2;
    std::future<void> fut1 = prom1.get_future(), fut2 = prom2.get_future();
    EXPECT_CALL(*queue, get_next_script()).Times(3).WillRepeatedly(Return(script));
    EXPECT_CALL(*queue, cancel_script()).Times(2).WillOnce(SetFuture(&prom1)).WillOnce(Return());
    EXPECT_CALL(*queue, enqueue_script(_, ans)).Times(1).WillOnce(SetFuture(&prom2));

    auto client = client_impl(stats, client_io_ctx, std::move(queue), server_host, server_port);

    ASSERT_TRUE(client.is_connected());

    stop_server();

    //The server sometimes gets too much time to be down, making the test unstable.
    std::this_thread::sleep_for(500ms);

    client.send();
    ASSERT_EQ(fut1.wait_for(1s), std::future_status::ready);
    ASSERT_FALSE(client.is_connected());

    start_server();

    std::condition_variable cv;
    std::mutex mtx;

    auto wait_for_connection = [&cv, &mtx](client_impl &c) {
        std::unique_lock lock(mtx);
        return cv.wait_for(lock, 2s, [&c] { return c.is_connected(); });
    };

    client.send();

    ASSERT_TRUE(wait_for_connection(client));

    client.send();
    ASSERT_EQ(fut2.wait_for(1s), std::future_status::ready);
}

}  // namespace http2_client
