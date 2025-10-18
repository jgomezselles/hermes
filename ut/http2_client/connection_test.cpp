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
    connection_test()
        : server_host("localhost"), server_port("8080"), server_started(false), is_secure(false) {};

    void start_server()
    {
        boost::system::error_code server_error_code;
        server = std::make_unique<nghttp2::asio_http2::server::http2>();

        if (is_secure)
        {
            boost::system::error_code ec;
            tlsCtx = std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
            tlsCtx->use_private_key_file("/usr/local/share/ca-certificates/localhost.key",
                                         boost::asio::ssl::context::pem);
            tlsCtx->use_certificate_chain_file("/usr/local/share/ca-certificates/localhost.crt");
            nghttp2::asio_http2::server::configure_tls_context_easy(ec, *tlsCtx);

            if (server->listen_and_serve(server_error_code, *tlsCtx, server_host, server_port,
                                         true))
            {
                fprintf(stderr, "Error starting server in %s:%s", server_host.c_str(),
                        server_port.c_str());
            }
        }
        else
        {
            if (server->listen_and_serve(server_error_code, server_host, server_port, true))
            {
                fprintf(stderr, "Error starting server in %s:%s", server_host.c_str(),
                        server_port.c_str());
            }
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
    std::unique_ptr<boost::asio::ssl::context> tlsCtx;
    const std::string server_host;
    const std::string server_port;
    bool server_started;
    bool is_secure;
};

class connection_test_p : public connection_test, public testing::WithParamInterface<bool>
{
public:
    connection_test_p() : connection_test() { is_secure = GetParam(); };
};

INSTANTIATE_TEST_CASE_P(is_secure, connection_test_p, testing::Values(false, true));

TEST_P(connection_test_p, correct_initialization)
{
    connection c(server_host, server_port, GetParam());
    ASSERT_TRUE(c.wait_to_be_connected());
    ASSERT_EQ(connection::status::OPEN, c.get_status());
}

TEST_P(connection_test_p, wrong_port)
{
    testing::internal::CaptureStderr();

    const std::string wrong_port = "1234";
    connection c(server_host, wrong_port, GetParam());

    ASSERT_FALSE(c.wait_to_be_connected());
    ASSERT_EQ(connection::status::CLOSED, c.get_status());

    const std::string expected_output{"Error in connection to localhost:" + wrong_port +
                                      " Message: Connection refused\n"};
    ASSERT_EQ(expected_output, testing::internal::GetCapturedStderr());
}

TEST_P(connection_test_p, connection_is_lost_because_of_the_server)
{
    connection c(server_host, server_port, GetParam());

    ASSERT_TRUE(c.wait_to_be_connected());
    ASSERT_EQ(connection::status::OPEN, c.get_status());
    stop_server();
    ASSERT_TRUE(c.wait_for_status(100ms, connection::status::CLOSED));
}

TEST_P(connection_test_p, close_connection)
{
    connection c(server_host, server_port, GetParam());

    ASSERT_TRUE(c.wait_to_be_connected());
    ASSERT_EQ(connection::status::OPEN, c.get_status());

    // This test is unstable due to, despite the efforts on checking the status,
    // the connection is not ready. Closing a connection which is not fully open
    // ends up in the nghttp2 client in the following error:
    // Assertion failed: !inside_callback_ (asio_client_session_impl.cc: enter_callback: 611)
    std::this_thread::sleep_for(100ms);
    c.close();

    ASSERT_FALSE(c.wait_to_be_connected());
    ASSERT_EQ(connection::status::CLOSED, c.get_status());
}
}  // namespace http2_client
