#include <gtest/gtest.h>

#include <atomic>
#include <boost/asio.hpp>
#include <chrono>
#include <functional>
#include <thread>

#include "timer_impl.hpp"

using namespace std::chrono_literals;
namespace ba = boost::asio;

class timer_test : public testing::Test
{
public:
    timer_test() : io_ctx(), guard(ba::make_work_guard(io_ctx))
    {
        worker = std::thread([this]() { io_ctx.run(); });
    };

    void TearDown() override
    {
        guard.reset();
        worker.join();
    };

protected:
    ba::io_context io_ctx;
    ba::executor_work_guard<ba::io_context::executor_type> guard;
    std::thread worker;
};

TEST_F(timer_test, TimerOnlyExecutesActionAfterExpiryTime)
{
    std::condition_variable cv;
    std::mutex mtx;
    std::atomic<bool> executed{false};

    engine::timer_impl t(io_ctx);
    t.expires_after(150ms);

    t.async_wait([&cv, &executed](const boost::system::error_code&) {
        executed.store(true);
        cv.notify_one();
    });

    auto wait_execution = [&cv, &mtx, &executed](std::chrono::milliseconds d) {
        std::unique_lock lock(mtx);
        return cv.wait_for(lock, d, [&executed] { return executed.load(); });
    };

    ASSERT_FALSE(wait_execution(100ms));
    ASSERT_TRUE(wait_execution(100ms));
}
