#include "script_queue.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <map>
#include <sstream>

#include "action_builder.hpp"
#include "json_element_builder.hpp"
#include "json_object_builder.hpp"
#include "message_builder.hpp"
#include "range_builder.hpp"
#include "response_builder.hpp"
#include "script.hpp"
#include "script_builder.hpp"

class script_queue_sut : public traffic::script_queue
{
public:
    script_queue_sut(const traffic::script& s) : traffic::script_queue(s) {}
    int64_t get_current(const std::string& range) { return current_in_range[range]; }
};

class script_queue_test : public ::testing::Test
{
protected:
    void setup_queue(const std::string& json)
    {
        std::stringstream json_stream;
        json_stream << json;
        traffic::script script(json_stream);
        script_queue = std::make_unique<script_queue_sut>(script);
    }

    std::unique_ptr<script_queue_sut> script_queue;
    ut_helpers::script_builder script_builder;
};

TEST_F(script_queue_test, EnqueueSimpleScriptJustRunOnce)
{
    setup_queue(script_builder.build());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt.is_initialized());
    EXPECT_TRUE(script_queue->has_pending_scripts());
    EXPECT_FALSE(script_queue->is_window_closed());

    script_queue->enqueue_script(script_opt.value(), {200, "OK"});
    EXPECT_FALSE(script_queue->has_pending_scripts());

    script_queue->close_window();
    EXPECT_TRUE(script_queue->is_window_closed());

    script_opt = script_queue->get_next_script();
    ASSERT_FALSE(script_opt.is_initialized());
}

TEST_F(script_queue_test, EnqueueMultipleMessageScriptRunTwice)
{
    setup_queue(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1).build(),
                                                       ut_helpers::message_builder(2).build()})
                    .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
                    .build());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt.is_initialized());
    EXPECT_TRUE(script_queue->has_pending_scripts());
    EXPECT_FALSE(script_queue->is_window_closed());

    script_queue->enqueue_script(script_opt.value(), {200, "OK"});
    EXPECT_TRUE(script_queue->has_pending_scripts());
    script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt.is_initialized());
    EXPECT_TRUE(script_queue->has_pending_scripts());
    EXPECT_FALSE(script_queue->is_window_closed());
    script_queue->enqueue_script(script_opt.value(), {200, "OK"});
    EXPECT_FALSE(script_queue->has_pending_scripts());

    script_queue->close_window();
    EXPECT_TRUE(script_queue->is_window_closed());

    script_opt = script_queue->get_next_script();
    ASSERT_FALSE(script_opt.is_initialized());
}

TEST_F(script_queue_test, CancelScriptReturnsNoMorePending)
{
    setup_queue(script_builder.build());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt.is_initialized());
    EXPECT_TRUE(script_queue->has_pending_scripts());

    script_queue->cancel_script();
    EXPECT_FALSE(script_queue->has_pending_scripts());

    script_opt = script_queue->get_next_script();
    EXPECT_TRUE(script_opt.is_initialized());
}

TEST_F(script_queue_test, CloseWindowReturnsNone)
{
    setup_queue(script_builder.build());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt.is_initialized());
    EXPECT_TRUE(script_queue->has_pending_scripts());
    EXPECT_FALSE(script_queue->is_window_closed());

    script_queue->close_window();
    script_opt = script_queue->get_next_script();
    EXPECT_TRUE(script_queue->has_pending_scripts());
    EXPECT_FALSE(script_opt.is_initialized());
    EXPECT_TRUE(script_queue->is_window_closed());
}

TEST_F(script_queue_test, CloseWindowAndCancelCurrentReturnsNoneAndNoMorePending)
{
    setup_queue(script_builder.build());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt.is_initialized());
    EXPECT_TRUE(script_queue->has_pending_scripts());
    EXPECT_FALSE(script_queue->is_window_closed());

    script_queue->close_window();
    script_queue->cancel_script();

    EXPECT_FALSE(script_queue->has_pending_scripts());

    script_opt = script_queue->get_next_script();
    EXPECT_FALSE(script_opt.is_initialized());
    EXPECT_TRUE(script_queue->is_window_closed());
}

TEST_F(script_queue_test, EnqueueMultipleMessageScriptAndRangesRunTwice)
{
    setup_queue(
        script_builder
            .ranges(std::vector<std::string>{ut_helpers::range_builder().min(5).max(6).build()})
            .messages(std::vector<std::string>{ut_helpers::message_builder(1).build(),
                                               ut_helpers::message_builder(2).build()})
            .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
            .build());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt.is_initialized());
    EXPECT_EQ(5, script_queue->get_current("range1"));

    // Here you will get a new one, because you did not enqueue the answer!
    auto script_opt2 = script_queue->get_next_script();
    EXPECT_EQ(6, script_queue->get_current("range1"));

    // Now let's answer the first one, so get_next_script will return
    // the first one back to us, keeping the "5"
    script_queue->enqueue_script(script_opt.value(), {200, "OK"});
    script_opt = script_queue->get_next_script();
    EXPECT_EQ(6, script_queue->get_current("range1"));

    // The same for script 2
    script_queue->enqueue_script(script_opt2.value(), {200, "OK"});
    script_opt = script_queue->get_next_script();
    EXPECT_EQ(6, script_queue->get_current("range1"));

    script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt.is_initialized());
    EXPECT_EQ(5, script_queue->get_current("range1"));
}
