#include "script_queue.hpp"

#include <gtest/gtest.h>

class script_queue_sut : public traffic::script_queue
{
public:
    script_queue_sut(const traffic::script& s) : traffic::script_queue(s) {}
    int64_t get_current(const std::string& range) { return current_in_range[range]; }
};

class script_queue_test : public ::testing::Test
{
public:
    traffic::json_reader build_script()
    {
        traffic::json_reader json;
        json.set<std::string>("/dns", "public-dns");
        json.set<std::string>("/port", "8686");
        json.set<int>("/timeout", 2000);
        json.set<std::vector<std::string>>("/flow", {"test1"});
        json.set<std::string>("/messages/test1/url", "v1/test");
        json.set<traffic::json_reader>("/messages/test1/body", {"{}", ""});
        json.set<std::string>("/messages/test1/method", "GET");
        json.set<int>("/messages/test1/response/code", 200);
        return json;
    }

    void setup_queue(const traffic::json_reader& json)
    {
        script_queue = std::make_unique<script_queue_sut>(traffic::script(json));
    }

    std::unique_ptr<script_queue_sut> script_queue;
};

TEST_F(script_queue_test, EnqueueSimpleScriptJustRunOnce)
{
    setup_queue(build_script());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt);
    ASSERT_TRUE(script_queue->has_pending_scripts());
    ASSERT_FALSE(script_queue->is_window_closed());

    script_queue->enqueue_script(script_opt.value(), {200, "OK"});
    ASSERT_FALSE(script_queue->has_pending_scripts());

    script_queue->close_window();
    ASSERT_TRUE(script_queue->is_window_closed());

    script_opt = script_queue->get_next_script();
    ASSERT_FALSE(script_opt);
}

TEST_F(script_queue_test, EnqueueMultipleMessageScriptRunTwice)
{
    auto json = build_script();
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    setup_queue(json);
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt);
    ASSERT_TRUE(script_queue->has_pending_scripts());
    ASSERT_FALSE(script_queue->is_window_closed());

    script_queue->enqueue_script(script_opt.value(), {200, "OK"});
    ASSERT_TRUE(script_queue->has_pending_scripts());
    script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt);
    ASSERT_TRUE(script_queue->has_pending_scripts());
    ASSERT_FALSE(script_queue->is_window_closed());
    script_queue->enqueue_script(script_opt.value(), {200, "OK"});
    ASSERT_FALSE(script_queue->has_pending_scripts());

    script_queue->close_window();
    ASSERT_TRUE(script_queue->is_window_closed());

    script_opt = script_queue->get_next_script();
    ASSERT_FALSE(script_opt);
}

TEST_F(script_queue_test, CancelScriptReturnsNoMorePending)
{
    setup_queue(build_script());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt);
    ASSERT_TRUE(script_queue->has_pending_scripts());

    script_queue->cancel_script();
    ASSERT_FALSE(script_queue->has_pending_scripts());

    script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt);
}

TEST_F(script_queue_test, CloseWindowReturnsNone)
{
    setup_queue(build_script());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt);
    ASSERT_TRUE(script_queue->has_pending_scripts());
    ASSERT_FALSE(script_queue->is_window_closed());

    script_queue->close_window();
    script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_queue->has_pending_scripts());
    ASSERT_FALSE(script_opt);
    ASSERT_TRUE(script_queue->is_window_closed());
}

TEST_F(script_queue_test, CloseWindowAndCancelCurrentReturnsNoneAndNoMorePending)
{
    setup_queue(build_script());
    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt);
    ASSERT_TRUE(script_queue->has_pending_scripts());
    ASSERT_FALSE(script_queue->is_window_closed());

    script_queue->close_window();
    script_queue->cancel_script();

    ASSERT_FALSE(script_queue->has_pending_scripts());

    script_opt = script_queue->get_next_script();
    ASSERT_FALSE(script_opt);
    ASSERT_TRUE(script_queue->is_window_closed());
}

TEST_F(script_queue_test, EnqueueMultipleMessageScriptAndRangesRunTwice)
{
    auto json = build_script();
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    json.set<int>("/ranges/range1/min", 5);
    json.set<int>("/ranges/range1/max", 6);
    setup_queue(json);

    auto script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt);
    ASSERT_EQ(5, script_queue->get_current("range1"));

    // Here you will get a new one, because you did not enqueue the answer!
    auto script_opt2 = script_queue->get_next_script();
    ASSERT_EQ(6, script_queue->get_current("range1"));

    // Now let's answer the first one, so get_next_script will return
    // the first one back to us, keeping the "5"
    script_queue->enqueue_script(script_opt.value(), {200, "OK"});
    script_opt = script_queue->get_next_script();
    ASSERT_EQ(6, script_queue->get_current("range1"));

    // The same for script 2
    script_queue->enqueue_script(script_opt2.value(), {200, "OK"});
    script_opt = script_queue->get_next_script();
    ASSERT_EQ(6, script_queue->get_current("range1"));

    script_opt = script_queue->get_next_script();
    ASSERT_TRUE(script_opt);
    ASSERT_EQ(5, script_queue->get_current("range1"));
}
