#include "script.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>
#include <sstream>

class script_test : public ::testing::Test
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
};

TEST_F(script_test, InvalidPath)
{
    ASSERT_THROW(traffic::script("/impossible/path/to/find.json"), std::logic_error);
}

TEST_F(script_test, EmptyFile)
{
    ASSERT_THROW(traffic::script("/dev/null"), std::logic_error);
}

TEST_F(script_test, MinimumCorrectFile)
{
    std::unique_ptr<traffic::script> script;
    ASSERT_NO_THROW(script = std::make_unique<traffic::script>(build_script()));
    EXPECT_EQ("public-dns", script->get_server_dns());
    EXPECT_EQ("8686", script->get_server_port());
    EXPECT_EQ(2000, script->get_timeout_ms());
    EXPECT_FALSE(script->is_server_secure());
    EXPECT_THAT(std::vector<std::string>{"test1"},
                testing::ContainerEq(script->get_message_names()));
}

TEST_F(script_test, SecureSetToFalse)
{
    auto json = build_script();
    json.set<bool>("/secure", false);
    std::unique_ptr<traffic::script> script;
    ASSERT_NO_THROW(script = std::make_unique<traffic::script>(json));
    ASSERT_FALSE(script->is_server_secure());
}

TEST_F(script_test, SecureSetToTrue)
{
    auto json = build_script();
    json.set<bool>("/secure", true);
    std::unique_ptr<traffic::script> script;
    ASSERT_NO_THROW(script = std::make_unique<traffic::script>(json));
    ASSERT_TRUE(script->is_server_secure());
}

TEST_F(script_test, PostProcessLastMessageReturnsFalse)
{
    const auto json = build_script();
    traffic::script script{json};
    ASSERT_FALSE(script.post_process(traffic::answer_type(200, "OK")));
}

TEST_F(script_test, PostProcessTwoAnswers)
{
    auto json = build_script();
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    traffic::script script{json};
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, "OK")));
    ASSERT_FALSE(script.post_process(traffic::answer_type(200, "OK")));
}

TEST_F(script_test, PostProcessFoundStringInSFAPath)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_string");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "string");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<std::string>(expected_path, "I am a string");
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, answer.as_string())));
}

TEST_F(script_test, PostProcessFoundIntInSFAPath)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_int");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "int");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<int>(expected_path, 7);
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, answer.as_string())));
}

TEST_F(script_test, PostProcessFoundObjectInSFAPath)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_object");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "object");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<traffic::json_reader>(expected_path, {"{}", "{}"});
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, answer.as_string())));
}

TEST_F(script_test, PostProcessFoundIntWhenExpectingStringValueInSFA)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_string");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "string");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<int>(expected_path, 123456);
    ASSERT_FALSE(script.post_process(traffic::answer_type(200, answer.as_string())));
}

TEST_F(script_test, PostProcessFoundStringWhenExpectingIntValueInSFA)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_string");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "int");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<std::string>(expected_path, "Oops, I'm a string");
    ASSERT_FALSE(script.post_process(traffic::answer_type(200, answer.as_string())));
}

TEST_F(script_test, PostProcessFoundStringWhenExpectingObjectValueInSFA)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_string");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "object");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<std::string>(expected_path, "Oops, I'm a string");
    ASSERT_FALSE(script.post_process(traffic::answer_type(200, answer.as_string())));
}

TEST_F(script_test, PostProcessCorrectStringValueInSFAUsedInATB)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_string");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "string");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_string");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", expected_path);
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "string");
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<std::string>(expected_path, "I am a string");
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, answer.as_string())));
    ASSERT_EQ(answer.as_string(), script.get_next_body());
}

TEST_F(script_test, PostProcessCorrectIntValueINSFAUsedInATB)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_int");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "int");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_int");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", expected_path);
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "int");
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<int>(expected_path, 53);
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, answer.as_string())));
    ASSERT_EQ(answer.as_string(), script.get_next_body());
}

TEST_F(script_test, PostProcessCorrectObjectValueInSFAUsedInATB)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_object");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "object");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_object");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", expected_path);
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "object");
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<std::string>(expected_path + "/sub_path1", "hi there");
    answer.set<int>(expected_path + "/sub_path1", 235);
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, answer.as_string())));
    ASSERT_EQ(answer.as_string(), script.get_next_body());
}

TEST_F(script_test, PostProcessNotFoundValueInSFAToUseInATB)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_int");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "int");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_int");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", expected_path);
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "int");
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<int>("/not/expected/path", 53);
    ASSERT_FALSE(script.post_process(traffic::answer_type(200, answer.as_string())));
}

TEST_F(script_test, PostProcessInCorrectTypeValueInSFAUsedInATB)
{
    const std::string expected_path{"/some/path"};
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_string");
    json.set<std::string>("/messages/test1/save_from_answer/path", expected_path);
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "string");
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_string");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", expected_path);
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "int");
    traffic::script script{json};

    traffic::json_reader answer;
    answer.set<std::string>(expected_path, "I am a string BUT I am expected to be int");
    ASSERT_FALSE(script.post_process(traffic::answer_type(200, answer.as_string())));
}

TEST_F(script_test, ParseRangesInRangeValue)
{
    auto json = build_script();
    json.set<int>("/ranges/my_range/min", 50);
    json.set<int>("/ranges/my_range/max", 60);
    json.set<std::string>("/messages/test1/url", "/my/url/<my_range>");
    json.set<std::string>("/messages/test1/body/data", "in-range-<my_range>");

    traffic::script script{json};
    script.parse_ranges(std::map<std::string, int64_t>{{"my_range", 55}});
    ASSERT_EQ("/my/url/55", script.get_next_url());
    ASSERT_EQ("{\"data\":\"in-range-55\"}", script.get_next_body());
}

TEST_F(script_test, SameNameInRangesAndVariables)
{
    auto json = build_script();
    json.set<int>("/ranges/my_range/min", 1);
    json.set<int>("/ranges/my_range/max", 2);
    json.set<int>("/variables/my_range", 4);

    ASSERT_THROW(traffic::script script{json}, std::logic_error);
}

TEST_F(script_test, ParseVariables)
{
    auto json = build_script();
    json.set<int>("/variables/my_int", 50);
    json.set<std::string>("/variables/my_string", "hello");
    json.set<std::string>("/messages/test1/url", "/my/<my_int>/path");
    json.set<traffic::json_reader>("/messages/test1/body",
                                   traffic::json_reader("{\"<my_string>\": true}", "{}"));

    traffic::script script{json};
    script.parse_variables();
    ASSERT_EQ("/my/50/path", script.get_next_url());
    ASSERT_EQ("{\"hello\":true}", script.get_next_body());
}

TEST_F(script_test, BuildAndGetNextHeadersOk)
{
    auto json = build_script();
    json.set<traffic::json_reader>(
        "/messages/test1/headers",
        traffic::json_reader(R"( { "key1" : "val1", "key2": "val2" } )", "{}"));

    traffic::script script{json};
    traffic::msg_headers h{ {"key1", "val1"}, {"key2", "val2"} };
    ASSERT_EQ( h, script.get_next_headers());
}

TEST_F(script_test, EmptyHeaders)
{
    auto json = build_script();
    traffic::script script{json};
    ASSERT_TRUE(script.get_next_headers().empty());
}