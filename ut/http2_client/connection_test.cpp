#ifndef UT_HTTP2_CLIENT_CONNECTION_CPP
#define UT_HTTP2_CLIENT_CONNECTION_CPP

#include "connection.hh"

#include <gtest/gtest.h>
#include <nghttp2/asio_http2_server.h>

#include <boost/system/error_code.hpp>
#include <chrono>
#include <thread>

namespace http2_client
{
class connection_test : public ::testing::Test
{
public:
    connection_test() : host("localhost"), port("8080"), server_started(false){};

    void start_server()
    {
        boost::system::error_code server_error_code;

        server = std::make_unique<nghttp2::asio_http2::server::http2>();

        if (server->listen_and_serve(server_error_code, host, port, true))
        {
            fprintf(stderr, "Error starting server in %s:%s", host.c_str(), port.c_str());
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

    const std::pair<bool, enum connection::status> connect_client_to_server(
        const std::string& dest_host, const std::string& dest_port)
    {
        sut_ptr = std::make_unique<connection>(dest_host, dest_port);

        const auto connected = sut_ptr->wait_to_be_connected();
        const auto sut_status = sut_ptr->get_status();

        return {connected, sut_status};
    }

    void SetUp() override
    {
        start_server();
        sut_ptr = std::make_unique<connection>(host, port);
    };

    void TearDown() override
    {
        sut_ptr.reset();
        if (server_started)
        {
            stop_server();
        }
    };

protected:
    std::unique_ptr<nghttp2::asio_http2::server::http2> server;
    std::unique_ptr<connection> sut_ptr;
    const std::string host;
    const std::string port;
    bool server_started;
};

TEST_F(connection_test, correct_initialization)
{
    // SETUP
    const auto expected_status = connection::status::OPEN;

    // EXEC
    const auto connected = sut_ptr->wait_to_be_connected();
    const auto sut_status = sut_ptr->get_status();

    // ASSERT
    ASSERT_TRUE(connected);
    ASSERT_EQ(expected_status, sut_status);
}

TEST_F(connection_test, incorrect_initialization_because_wrong_port)
{
    // SETUP
    const auto expected_status = connection::status::CLOSED;
    const std::string wrong_port = "1234";
    const std::string expected_output{"Error in connection to localhost:" + wrong_port +
                                      " Message: Connection refused\n"};
    sut_ptr.reset();

    // EXEC
    testing::internal::CaptureStderr();
    const auto& [connected, sut_status] = connect_client_to_server(host, wrong_port);
    const auto output_error = testing::internal::GetCapturedStderr();

    // ASSERT
    ASSERT_FALSE(connected);
    ASSERT_EQ(expected_status, sut_status);
    ASSERT_EQ(expected_output, output_error);
}

TEST_F(connection_test, incorrect_initialization_wrong_host)
{
    // SETUP
    const auto expected_status = connection::status::CLOSED;
    const std::string wrong_host = "localhostus";
    const std::string expected_output{"Error in connection to " + wrong_host +
                                      ":8080 Message: Host not found (authoritative)\n"};
    sut_ptr.reset();

    // EXEC
    testing::internal::CaptureStderr();
    const auto& [connected, sut_status] = connect_client_to_server(wrong_host, port);
    const auto output_error = testing::internal::GetCapturedStderr();

    // ASSERT
    ASSERT_FALSE(connected);
    ASSERT_EQ(expected_status, sut_status);
    ASSERT_EQ(expected_output, output_error);
}

TEST_F(connection_test, connection_is_lost_because_of_the_server)
{
    // SETUP
    const auto expected_initial_status = connection::status::OPEN;
    const auto expected_final_status = connection::status::CLOSED;

    // EXEC
    auto connected = sut_ptr->wait_to_be_connected();
    auto sut_status = sut_ptr->get_status();

    EXPECT_TRUE(connected);
    EXPECT_EQ(expected_initial_status, sut_status);

    testing::internal::CaptureStderr();

    stop_server();
    // this sucks, but better have a reliable test rather that one which fails
    // every X random times. Please come with better ideas
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    connected = sut_ptr->wait_to_be_connected();
    sut_status = sut_ptr->get_status();

    // ASSERT
    ASSERT_FALSE(connected);
    ASSERT_EQ(expected_final_status, sut_status);
}

TEST_F(connection_test, close_connection)
{
    // SETUP
    const auto expected_initial_status = connection::status::OPEN;
    const auto expected_final_status = connection::status::CLOSED;

    // EXEC
    auto connected = sut_ptr->wait_to_be_connected();
    auto sut_status = sut_ptr->get_status();

    EXPECT_TRUE(connected);
    EXPECT_EQ(expected_initial_status, sut_status);

    sut_ptr->close();

    connected = sut_ptr->wait_to_be_connected();
    sut_status = sut_ptr->get_status();

    // ASSERT
    ASSERT_FALSE(connected);
    ASSERT_EQ(expected_final_status, sut_status);
}
}  // namespace http2_client
#endif