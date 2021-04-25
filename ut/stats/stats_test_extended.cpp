#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>

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
          msg_names({"msg1", "msg2", "msg3"}),
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
        testing::internal::CaptureStdout();
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
        std::remove("stats_test_extended.msg3");
        testing::internal::GetCapturedStdout();
    };

    std::vector<std::string> read_file(const std::string& filename)
    {
        std::vector<std::string> lines;
        std::ifstream file(filename);
        if (!file)
        {
            return lines;
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (!line.size())
            {
                continue;
            }
            lines.push_back(line);
        }

        file.close();

        return lines;
    }

    std::string formatted_output(const float time, const int sent, const int ok, const int rt,
                                 const int nok, const int timeout)
    {
        std::stringstream ss;
        ss << std::left << std::setw(10) << time << std::right << std::setw(10)
           << float(sent) / time << std::right << std::setw(10) << float(ok) / time << std::right
           << std::setw(15) << rt << std::right << std::setw(15) << rt << std::right
           << std::setw(15) << rt <<

            std::right << std::setw(15) << sent << std::right << std::setw(15) << ok << std::right
           << std::setw(15) << nok << std::right << std::setw(15) << timeout;

        return ss.str();
    }

    float extract_real_time_from_line(const std::string& line)
    {
        std::string time;
        std::getline(std::stringstream(line), time, ' ');
        return atof(time.c_str());
    }

protected:
    ba::io_context io_ctx;
    ba::executor_work_guard<ba::io_context::executor_type> guard;
    std::vector<std::thread> workers;

    std::string output_file_name;
    std::vector<std::string> msg_names;
    stats_extended_sut sut;
    const std::string expected_headers =
        "Time (s)      Sent/s    Recv/s        RT (ms)     minRT (ms)     maxRT (ms)           "
        "Sent        Success         Errors       Timeouts";
};

TEST_F(stats_test_extended, PrintHeaders)
{
    testing::internal::CaptureStdout();

    sut.trigger_print_headers();

    ASSERT_EQ(testing::internal::GetCapturedStdout(), expected_headers + "\n");
}

TEST_F(stats_test_extended, PrintAndWrite)
{
    // This "uber-test" covers everything because it needs one second.
    for (int i{0}; i < 10; ++i)
    {
        sut.increase_sent("msg1");
        sut.add_measurement("msg1", 1000, 200);

        sut.increase_sent("msg2");
        sut.add_error("msg2", 500);

        sut.increase_sent("msg3");
        sut.add_timeout("msg3");
    }

    testing::internal::CaptureStdout();
    std::this_thread::sleep_for(1.1s);
    std::string expected_cout =
        "1.0             30.0      10.0          1.000          1.000          1.000             "
        "30             10             10             10";
    ASSERT_EQ(testing::internal::GetCapturedStdout(), expected_cout + "\n");

    const auto accum_content = read_file("stats_test_extended.accum");
    ASSERT_FALSE(accum_content.empty());
    ASSERT_EQ(expected_headers, accum_content.at(1));

    auto time_from_file = extract_real_time_from_line(accum_content.at(2));

    ASSERT_EQ(formatted_output(time_from_file, 30, 10, 1, 10, 10), accum_content.at(2));
}

}  // namespace stats
