#include <gtest/gtest.h>

#include <boost/asio.hpp>

#include "stats.hpp"

namespace ba = boost::asio;

namespace stats
{
class stats_extended_sut : public stats
{
public:
    stats_extended_sut(boost::asio::io_context& io_ctx, const int print_period,
                       const std::string& output_file_name,
                       const std::vector<std::string>& msg_names)
        : stats(io_ctx, print_period, output_file_name, msg_names){};

    void trigger_print_headers() const { print_headers(); };
};

class stats_test_extended : public testing::Test
{
public:
    stats_test_extended()
        : guard(ba::make_work_guard(io_ctx)),
          output_file_name("stats_test_extended"),
          msg_names({"msg1", "msg2"}),
          sut(io_ctx, 1, output_file_name, msg_names){};

    void SetUp() override
    {
        for (auto i = 0; i < 2; ++i)
        {
            workers.emplace_back([this]() { io_ctx.run(); });
        }
    };

    void TearDown() override
    {
        sut.end();
        guard.reset();
        for (auto& thread : workers)
        {
            thread.join();
        }

        std::remove("stats_test_extended.accum");
        std::remove("stats_test_extended.partial");
        std::remove("stats_test_extended.err");
        std::remove("stats_test_extended.msg1");
        std::remove("stats_test_extended.msg2");
    };

protected:
    ba::io_context io_ctx;
    ba::executor_work_guard<ba::io_context::executor_type> guard;
    std::vector<std::thread> workers;

    std::string output_file_name;
    std::vector<std::string> msg_names;
    stats_extended_sut sut;
};

TEST_F(stats_test_extended, WriteHeaders)
{
    testing::internal::CaptureStdout();

    sut.trigger_print_headers();

    std::string expected_headers =
        "Time (s)      Sent/s    Recv/s        RT (ms)     minRT (ms)     maxRT (ms)           "
        "Sent        Success         Errors       Timeouts\n";

    ASSERT_EQ(testing::internal::GetCapturedStdout(), expected_headers);
}

}  // namespace stats
