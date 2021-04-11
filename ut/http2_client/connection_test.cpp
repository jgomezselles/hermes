#include "connection.hpp"

#include <gtest/gtest.h>
#include <nghttp2/asio_http2_server.h>

#include <boost/system/error_code.hpp>

using namespace std::chrono_literals;

namespace http2_client
{
class connection_test : public ::testing::Test
{
public:
    connection_test() : server_host("localhost"), server_port("8080"), server_started(false){};

    void start_server()
    {
        boost::system::error_code server_error_code;

        server = std::make_unique<nghttp2::asio_http2::server::http2>();

        if (server->listen_and_serve(server_error_code, server_host, server_port, true))
        {
            fprintf(stderr, "Error starting server in %s:%s", server_host.c_str(),
                    server_port.c_str());
        }

        server_started = true;
    }

    void stop_server()
    {
        for (auto& service : server->io_services())
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
    };

protected:
    std::unique_ptr<nghttp2::asio_http2::server::http2> server;
    const std::string server_host;
    const std::string server_port;
    bool server_started;
};

TEST_F(connection_test, correct_initialization)
{
    connection c(server_host, server_port);
    ASSERT_TRUE(c.wait_to_be_connected());
    ASSERT_EQ(connection::status::OPEN, c.get_status());
}

TEST_F(connection_test, wrong_port)
{
    testing::internal::CaptureStderr();

    const std::string wrong_port = "1234";
    connection c(server_host, wrong_port);

    ASSERT_FALSE(c.wait_to_be_connected());
    ASSERT_EQ(connection::status::CLOSED, c.get_status());

    const std::string expected_output{"Error in connection to localhost:" + wrong_port +
                                      " Message: Connection refused\n"};
    ASSERT_EQ(expected_output, testing::internal::GetCapturedStderr());
}

TEST_F(connection_test, connection_is_lost_because_of_the_server)
{
    connection c(server_host, server_port);

    EXPECT_TRUE(c.wait_to_be_connected());
    EXPECT_EQ(connection::status::OPEN, c.get_status());
    stop_server();
    ASSERT_TRUE(c.wait_for_status(100ms, connection::status::CLOSED));
}

TEST_F(connection_test, close_connection)
{
    connection c(server_host, server_port);

    EXPECT_TRUE(c.wait_to_be_connected());
    EXPECT_EQ(connection::status::OPEN, c.get_status());

    c.close();

    ASSERT_FALSE(c.wait_to_be_connected());
    ASSERT_EQ(connection::status::CLOSED, c.get_status());
}
}  // namespace http2_client
