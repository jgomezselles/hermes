#include "sender.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "client.hpp"
#include "params.hpp"
#include "timer.hpp"

using namespace ::testing;

class timer_mock : public engine::timer
{
public:
    MOCK_METHOD1(async_wait, void(wait_handler&&));
    MOCK_METHOD1(expires_after, size_t(const duration_us));
};

class client_mock : public http2_client::client
{
public:
    MOCK_CONST_METHOD0(has_finished, bool());
    MOCK_METHOD0(send, void());
    MOCK_METHOD0(close_window, void());
    MOCK_CONST_METHOD0(is_connected, bool());
};

class sender_test : public ::testing::Test
{
public:
    sender_test() : client(new client_mock), timer(new timer_mock), fut(prom.get_future()) {}

    static void adjust_time(std::shared_ptr<config::params> params)
    {
        params->init_time = params->init_time - std::chrono::microseconds(params->wait_time);
    }

protected:
    std::unique_ptr<client_mock> client;
    std::unique_ptr<timer_mock> timer;
    std::promise<void> prom;
    std::future<void> fut;
};

TEST_F(sender_test, SimpleTest)
{
    auto params = std::make_shared<config::params>(1000000, 5);
    auto times = params->duration * 1000000 / params->wait_time;
    engine::timer::wait_handler handler;
    EXPECT_CALL(*timer, async_wait(_)).Times(times);
    EXPECT_CALL(*timer, expires_after(_)).Times(times - 1);

    EXPECT_CALL(*client, send()).Times(times - 1);
    EXPECT_CALL(*client, has_finished()).WillOnce(Return(true));
    EXPECT_CALL(*client, close_window()).Times(1);

    engine::sender sender(std::move(timer), std::move(client), params, std::move(prom));

    for (int i = 0; i < times; i++)
    {
        adjust_time(params);
        sender.send();
    }

    EXPECT_EQ(fut.wait_for(std::chrono::seconds(0)), std::future_status::ready);
}
