#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/asio.hpp>
#include <cstdio>
#include <map>
#include <sstream>

#include "stats.hpp"

namespace stats
{

class stats_sut : public stats
{
public:
    stats_sut(boost::asio::io_context& io_ctx, const int print_period,
              const std::string& output_file_name, const std::vector<std::string>& msg_names)
        : stats(io_ctx, print_period, output_file_name, msg_names){};

    const snapshot& get_total_snap() const { return total_snap; }

    const snapshot& get_partial_snap() const { return partial_snap; }

    const std::map<std::string, snapshot>& get_msg_snaps() const { return msg_snaps; }
};

class stats_test : public ::testing::TestWithParam<int>
{
public:
    stats_test()
        : print_period(1),
          output_file_name("stats_test_output"),
          msg_names({"msg1", "msg2"}),
          sut(io_ctx, print_period, output_file_name, msg_names){};

    void SetUp(){};

    void TearDown()
    {
        std::remove("stats_test_output.accum");
        std::remove("stats_test_output.partial");
        std::remove("stats_test_output.err");
        std::remove("stats_test_output.msg1");
        std::remove("stats_test_output.msg2");
    };

protected:
    boost::asio::io_context io_ctx;
    int print_period;
    std::string output_file_name;
    std::vector<std::string> msg_names;
    stats_sut sut;
};

INSTANTIATE_TEST_CASE_P(stats_test_parametrization, stats_test, ::testing::Values(1, 100));

/*
 * This test only checks that the extended struct works as intended
 */
TEST_P(stats_test, get_functions_ok)
{
    // SETUP
    const snapshot expected_total_snapshot;
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_total_snapshot},
                                                             {"msg2", expected_total_snapshot}};

    // EXEC
    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_total_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_total_snapshot, received_partial_snapshot);
    for (const auto& [id, snap] : received_msg_snaps)
    {
        EXPECT_EQ(expected_msg_snaps.at(id), snap);
    }
}

TEST_P(stats_test, increase_sent_existent_id)
{
    // SETUP
    const auto thread_number = GetParam();
    const snapshot expected_snapshot{
        thread_number,  // sent
        0,              // responded_ok
        0,              // timed_out
        0,              // rate
        0,              // avg_rt
        0,              // max_rt
        0,              // min_rt
        {},             // response_codes_ok
        {}              // response_codes_nok
    };
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_snapshot},
                                                             {"msg2", expected_snapshot}};

    std::vector<std::thread> threads;

    // EXEC
    for (int i = 0; i < thread_number; ++i)
    {
        threads.push_back(std::thread{[&, this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(thread_number > 1 ? 50 : 0));
            sut.increase_sent("msg1");
        }});
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_snapshot, received_partial_snapshot);
    EXPECT_EQ(expected_msg_snaps.at("msg1"), received_msg_snaps.at("msg1"));
}

TEST_P(stats_test, increase_sent_non_existent_id_exception)
{
    // SETUP
    // EXEC
    // ASSERT
    EXPECT_THROW(sut.increase_sent("non-existent"), std::exception);
}

TEST_P(stats_test, add_measurement_responded_ok_zero_min_rt_zero_max_rt_lower_than_elapsed_time)
{
    // SETUP
    const auto thread_number = GetParam();

    const int64_t elapsed_time{1};
    const int code{200};

    const snapshot expected_snapshot{
        0,                        // sent
        thread_number,            // responded_ok
        0,                        // timed_out
        0,                        // rate
        elapsed_time,             // avg_rt
        elapsed_time,             // max_rt
        elapsed_time,             // min_rt
        {{code, thread_number}},  // response_codes_ok
        {}                        // response_codes_nok
    };
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_snapshot},
                                                             {"msg2", expected_snapshot}};

    std::vector<std::thread> threads;

    // EXEC
    for (int i = 0; i < thread_number; ++i)
    {
        threads.push_back(std::thread{[&, this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(thread_number > 1 ? 50 : 0));
            sut.add_measurement("msg1", elapsed_time, code);
        }});
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_snapshot, received_partial_snapshot);
    EXPECT_EQ(expected_msg_snaps.at("msg1"), received_msg_snaps.at("msg1"));
}

TEST_P(stats_test, add_measurement_min_rt_and_max_rt_greater_than_elapsed_time)
{
    // SETUP
    const auto thread_number = GetParam();

    const int64_t elapsed_time{-1};
    const int code{200};

    const snapshot expected_snapshot{
        0,                        // sent
        thread_number,            // responded_ok
        0,                        // timed_out
        0,                        // rate
        elapsed_time,             // avg_rt
        0,                        // max_rt
        elapsed_time,             // min_rt
        {{code, thread_number}},  // response_codes_ok
        {}                        // response_codes_nok
    };
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_snapshot},
                                                             {"msg2", expected_snapshot}};

    std::vector<std::thread> threads;

    // EXEC
    for (int i = 0; i < thread_number; ++i)
    {
        threads.push_back(std::thread{[&, this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(thread_number > 1 ? 50 : 0));
            sut.add_measurement("msg1", elapsed_time, code);
        }});
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_snapshot, received_partial_snapshot);
    EXPECT_EQ(expected_msg_snaps.at("msg1"), received_msg_snaps.at("msg1"));
}

TEST_P(stats_test, add_measurement_updates_response_codes_ok_and_responded_ok_greater_than_1)
{
    // SETUP
    const auto thread_number = GetParam();

    const int64_t elapsed_time{-1};
    const int code{200};

    const snapshot expected_snapshot{
        0,                            // sent
        thread_number * 2,            // responded_ok
        0,                            // timed_out
        0,                            // rate
        elapsed_time,                 // avg_rt
        0,                            // max_rt
        elapsed_time,                 // min_rt
        {{code, thread_number * 2}},  // response_codes_ok
        {}                            // response_codes_nok
    };
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_snapshot},
                                                             {"msg2", expected_snapshot}};

    std::vector<std::thread> threads;

    // EXEC
    for (int i = 0; i < thread_number; ++i)
    {
        threads.push_back(std::thread{[&, this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(thread_number > 1 ? 50 : 0));
            sut.add_measurement("msg1", elapsed_time, code);
            sut.add_measurement("msg1", elapsed_time, code);
        }});
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_snapshot, received_partial_snapshot);
    EXPECT_EQ(expected_msg_snaps.at("msg1"), received_msg_snaps.at("msg1"));
}

TEST_P(stats_test, add_timeout_ok)
{
    // SETUP
    const auto thread_number = GetParam();

    const snapshot expected_snapshot{
        0,              // sent
        0,              // responded_ok
        thread_number,  // timed_out
        0,              // rate
        0,              // avg_rt
        0,              // max_rt
        0,              // min_rt
        {},             // response_codes_ok
        {}              // response_codes_nok
    };
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_snapshot},
                                                             {"msg2", expected_snapshot}};

    std::vector<std::thread> threads;

    // EXEC
    for (int i = 0; i < thread_number; ++i)
    {
        threads.push_back(std::thread{[&, this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(thread_number > 1 ? 50 : 0));
            sut.add_timeout("msg1");
        }});
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_snapshot, received_partial_snapshot);
    EXPECT_EQ(expected_msg_snaps.at("msg1"), received_msg_snaps.at("msg1"));
}

TEST_P(stats_test, add_timeout_id_non_existent)
{
    // SETUP
    // EXEC
    // ASSERT
    EXPECT_THROW(sut.add_timeout("non-existent"), std::exception);
}

TEST_P(stats_test, add_error_ok)
{
    // SETUP
    const auto thread_number = GetParam();

    const int error{500};

    const snapshot expected_snapshot{
        0,                        // sent
        0,                        // responded_ok
        0,                        // timed_out
        0,                        // rate
        0,                        // avg_rt
        0,                        // max_rt
        0,                        // min_rt
        {},                       // response_codes_ok
        {{error, thread_number}}  // response_codes_nok
    };
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_snapshot},
                                                             {"msg2", expected_snapshot}};

    std::vector<std::thread> threads;

    // EXEC
    for (int i = 0; i < thread_number; ++i)
    {
        threads.push_back(std::thread{[&, this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(thread_number > 1 ? 50 : 0));
            sut.add_error("msg1", error);
        }});
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_snapshot, received_partial_snapshot);
    EXPECT_EQ(expected_msg_snaps.at("msg1"), received_msg_snaps.at("msg1"));
}

TEST_P(stats_test, add_error_updates_existent_error)
{
    // SETUP
    const auto thread_number = GetParam();

    const int error{500};

    const snapshot expected_snapshot{
        0,                            // sent
        0,                            // responded_ok
        0,                            // timed_out
        0,                            // rate
        0,                            // avg_rt
        0,                            // max_rt
        0,                            // min_rt
        {},                           // response_codes_ok
        {{error, thread_number * 2}}  // response_codes_nok
    };
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_snapshot},
                                                             {"msg2", expected_snapshot}};

    std::vector<std::thread> threads;

    // EXEC
    for (int i = 0; i < thread_number; ++i)
    {
        threads.push_back(std::thread{[&, this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(thread_number > 1 ? 50 : 0));
            sut.add_error("msg1", error);
            sut.add_error("msg1", error);
        }});
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_snapshot, received_partial_snapshot);
    EXPECT_EQ(expected_msg_snaps.at("msg1"), received_msg_snaps.at("msg1"));
}

TEST_P(stats_test, add_error_id_non_existant)
{
    // SETUP
    // EXEC
    // ASSERT
    EXPECT_THROW(sut.add_error("non-existent", 0), std::exception);
}

TEST_P(stats_test, add_client_error_ok)
{
    // SETUP
    const auto thread_number = GetParam();

    const int error{500};

    const snapshot expected_snapshot{
        thread_number,            // sent
        0,                        // responded_ok
        0,                        // timed_out
        0,                        // rate
        0,                        // avg_rt
        0,                        // max_rt
        0,                        // min_rt
        {},                       // response_codes_ok
        {{error, thread_number}}  // response_codes_nok
    };
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_snapshot},
                                                             {"msg2", expected_snapshot}};

    std::vector<std::thread> threads;

    // EXEC
    for (int i = 0; i < thread_number; ++i)
    {
        threads.push_back(std::thread{[&, this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(thread_number > 1 ? 50 : 0));
            sut.add_client_error("msg1", error);
        }});
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_snapshot, received_partial_snapshot);
    EXPECT_EQ(expected_msg_snaps.at("msg1"), received_msg_snaps.at("msg1"));
}

TEST_P(stats_test, add_client_error_updates_existent_error)
{
    // SETUP
    const auto thread_number = GetParam();

    const int error{500};

    const snapshot expected_snapshot{
        thread_number * 2,            // sent
        0,                            // responded_ok
        0,                            // timed_out
        0,                            // rate
        0,                            // avg_rt
        0,                            // max_rt
        0,                            // min_rt
        {},                           // response_codes_ok
        {{error, thread_number * 2}}  // response_codes_nok
    };
    const std::map<std::string, snapshot> expected_msg_snaps{{"msg1", expected_snapshot},
                                                             {"msg2", expected_snapshot}};

    std::vector<std::thread> threads;

    // EXEC
    for (int i = 0; i < thread_number; ++i)
    {
        threads.push_back(std::thread{[&, this] {
            std::this_thread::sleep_for(std::chrono::milliseconds(thread_number > 1 ? 50 : 0));
            sut.add_client_error("msg1", error);
            sut.add_client_error("msg1", error);
        }});
    }
    for (auto& thread : threads)
    {
        thread.join();
    }

    const auto& received_total_snapshot = sut.get_total_snap();
    const auto& received_partial_snapshot = sut.get_partial_snap();
    const auto& received_msg_snaps = sut.get_msg_snaps();

    // ASSERT
    EXPECT_EQ(expected_snapshot, received_total_snapshot);
    EXPECT_EQ(expected_snapshot, received_partial_snapshot);
    EXPECT_EQ(expected_msg_snaps.at("msg1"), received_msg_snaps.at("msg1"));
}

TEST_P(stats_test, add_client_error_id_non_existant)
{
    // SETUP
    // EXEC
    // ASSERT
    EXPECT_THROW(sut.add_client_error("non-existent", 0), std::exception);
}
}  // namespace stats
