#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <filesystem>
#include <fstream>
#include <thread>

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
        : stats(io_ctx, print_period, output_file_name, msg_names) {};

    void trigger_print_headers() const { print_headers(); };
};

class stats_test_extended : public testing::Test
{
public:
    stats_test_extended()
        : guard(ba::make_work_guard(io_ctx)),
          output_file_name("stats_test_extended"),
          msg_names({"msg1", "msg2", "msg3"}),
          sut(io_ctx, 1, output_file_name, msg_names) {};

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

    std::vector<float> extract_fields_from_line(const std::string& line)
    {
        auto ss = std::stringstream(line);
        std::string f_str;
        std::vector<float> numbers;
        while (std::getline(ss, f_str, ' '))
        {
            if (f_str.size() && f_str != " ")
            {
                numbers.push_back(atof(f_str.c_str()));
            }
        }
        return numbers;
    }

    void validate_fields(const std::string& line, const std::vector<float> expected)
    {
        auto numbers = extract_fields_from_line(line);
        ASSERT_EQ(numbers.size(), expected.size());

        for (unsigned int i = 0; i < expected.size(); ++i)
        {
            ASSERT_LE(fabs(expected.at(i) - numbers.at(i)), 1);
        }
    }

    void simulate_responses()
    {
        for (int i{0}; i < 10; ++i)
        {
            sut.increase_sent("msg1");
            sut.add_measurement("msg1", 1000, 200);

            sut.increase_sent("msg2");
            sut.add_error("msg2", 500);

            sut.increase_sent("msg3");
            sut.add_timeout("msg3");
        }
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
    simulate_responses();
    testing::internal::CaptureStdout();
    std::this_thread::sleep_for(1.1s);
    validate_fields(testing::internal::GetCapturedStdout(), {1, 30, 10, 1, 1, 1, 30, 10, 10, 10});

    simulate_responses();
    testing::internal::CaptureStdout();
    std::this_thread::sleep_for(1.1s);
    validate_fields(testing::internal::GetCapturedStdout(), {2, 30, 10, 1, 1, 1, 60, 20, 20, 20});

    // accum
    const auto accum_content = read_file("stats_test_extended.accum");
    ASSERT_FALSE(accum_content.empty());
    ASSERT_EQ(expected_headers, accum_content.at(1));
    validate_fields(accum_content.at(2), {1, 30, 10, 1, 1, 1, 30, 10, 10, 10});
    validate_fields(accum_content.at(3), {2, 30, 10, 1, 1, 1, 60, 20, 20, 20});

    // partial
    const auto partial_content = read_file("stats_test_extended.partial");
    ASSERT_FALSE(partial_content.empty());
    ASSERT_EQ(expected_headers, partial_content.at(1));
    validate_fields(partial_content.at(2), {1, 30, 10, 1, 1, 1, 30, 10, 10, 10});
    validate_fields(partial_content.at(3), {2, 30, 10, 1, 1, 1, 30, 10, 10, 10});

    // msg1
    const auto msg1_content = read_file("stats_test_extended.msg1");
    ASSERT_FALSE(msg1_content.empty());
    ASSERT_EQ(expected_headers, msg1_content.at(1));
    validate_fields(msg1_content.at(2), {1, 10, 10, 1, 1, 1, 10, 10, 0, 0});
    validate_fields(msg1_content.at(3), {2, 10, 10, 1, 1, 1, 20, 20, 0, 0});

    // msg2
    const auto msg2_content = read_file("stats_test_extended.msg2");
    ASSERT_FALSE(msg2_content.empty());
    ASSERT_EQ(expected_headers, msg2_content.at(1));
    validate_fields(msg2_content.at(2), {1, 10, 0, 0, 0, 0, 10, 0, 10, 0});
    validate_fields(msg2_content.at(3), {2, 10, 0, 0, 0, 0, 20, 0, 20, 0});

    // msg3
    const auto msg3_content = read_file("stats_test_extended.msg3");
    ASSERT_FALSE(msg3_content.empty());
    ASSERT_EQ(expected_headers, msg3_content.at(1));
    validate_fields(msg3_content.at(2), {1, 10, 0, 0, 0, 0, 10, 0, 0, 10});
    validate_fields(msg3_content.at(3), {2, 10, 0, 0, 0, 0, 20, 0, 0, 20});

    // err
    const auto err_content = read_file("stats_test_extended.err");
    ASSERT_FALSE(err_content.empty());
    validate_fields(err_content.at(2), {1, 500, 10});
    validate_fields(err_content.at(3), {2, 500, 20});
}

}  // namespace stats
