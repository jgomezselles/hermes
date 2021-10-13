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

protected:
    void buildStream(const std::string &json)
    {
        json_stream.clear();
        json_stream << json;
    }

    std::stringstream json_stream;
};

TEST_F(script_test, EmptyScript)
{
    ASSERT_THROW(traffic::script(""), std::logic_error);
}

TEST_F(script_test, NullScript)
{
    ASSERT_THROW(traffic::script(nullptr), std::logic_error);
}

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
    json_stream << build_script().as_string();

    std::unique_ptr<traffic::script> script;
    ASSERT_NO_THROW(script = std::make_unique<traffic::script>(json_stream));
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
    json_stream << json.as_string();

    std::unique_ptr<traffic::script> script;
    ASSERT_NO_THROW(script = std::make_unique<traffic::script>(json_stream));
    EXPECT_FALSE(script->is_server_secure());
}

TEST_F(script_test, SecureSetToTrue)
{
    auto json = build_script();
    json.set<bool>("/secure", true);
    json_stream << json.as_string();
    std::unique_ptr<traffic::script> script;
    ASSERT_NO_THROW(script = std::make_unique<traffic::script>(json_stream));
    EXPECT_TRUE(script->is_server_secure());
}

TEST_F(script_test, ValidationErrorNoFlow)
{
    auto json = build_script();
    json.erase("/flow");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoMessages)
{
    auto json = build_script();
    json.erase("/messages");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoDns)
{
    auto json = build_script();
    json.erase("/dns");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoPort)
{
    auto json = build_script();
    json.erase("/port");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoTimeout)
{
    auto json = build_script();
    json.erase("/timeout");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoMethodInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/method");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoUrlInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/url");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoBodyInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/body");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoResponseInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/response");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoCodeInResponseInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/response/code");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, TotalMessageIsNotErrorIfNotCalledInFlow)
{
    auto json = build_script();
    auto other_msg = json.get_value<traffic::json_reader>("/messages/test1");
    json.set<traffic::json_reader>("/messages/Total", other_msg);
    json_stream << json.as_string();
    ASSERT_NO_THROW(traffic::script{json_stream});
}

TEST_F(script_test, TotalMessageIsReserved)
{
    auto json = build_script();
    auto other_msg = json.get_value<traffic::json_reader>("/messages/test1");
    json.set<std::vector<std::string>>("/flow", {"Total", "test1"});
    json.set<traffic::json_reader>("/messages/Total", other_msg);
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageInFlowNotFound)
{
    auto json = build_script();
    json.set<std::vector<std::string>>("/flow", {"test1", "test2"});
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithSFATypes)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_string");
    json.set<std::string>("/messages/test1/save_from_answer/path", "/some/path");
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "string");

    json.set<std::string>("/messages/test1/save_from_answer/name", "my_object");
    json.set<std::string>("/messages/test1/save_from_answer/path", "/some/path");
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "object");

    json.set<std::string>("/messages/test1/save_from_answer/name", "my_int");
    json.set<std::string>("/messages/test1/save_from_answer/path", "/some/path");
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "int");

    json_stream << json.as_string();
    ASSERT_NO_THROW(traffic::script{json_stream});
}

TEST_F(script_test, MessageWithSFAWrongType)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_sfa");
    json.set<std::string>("/messages/test1/save_from_answer/path", "/some/path");
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "bool");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithSFANoName)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/path", "/some/path");
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "string");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithSFANoPath)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_sfa");
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "object");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithSFANoValueType)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_sfa");
    json.set<std::string>("/messages/test1/save_from_answer/path", "/some/path");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithAFSTBTypes)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_string");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", "/some/path");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "string");

    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_object");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", "/some/path");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "object");

    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_int");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", "/some/path");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "int");

    json_stream << json.as_string();
    ASSERT_NO_THROW(traffic::script{json_stream});
}

TEST_F(script_test, MessageWithAFSTBWrongType)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_sfa");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", "/some/path");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "bool");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithAFSTBNoName)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", "/some/path");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "string");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithAFSTBNoPath)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_sfa");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "object");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithAFSTBNoValueType)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_sfa");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", "/some/path");
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, BuildRanges)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/min", 0);
    json.set<int>("/ranges/range1/max", 1000);
    json.set<int>("/ranges/range2/min", 1);
    json.set<int>("/ranges/range2/max", 30);
    json_stream << json.as_string();
    ASSERT_NO_THROW(traffic::script{json_stream});
}

TEST_F(script_test, NoMinInRange)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/max", 2);
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, NoMaxInRange)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/min", 2);
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MaxLowerThanMinInRange)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/max", 2);
    json.set<int>("/ranges/range1/min", 3);
    json_stream << json.as_string();
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MaxEqualToMinInRange)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/max", 666);
    json.set<int>("/ranges/range1/min", 666);
    json_stream << json.as_string();
    ASSERT_NO_THROW(traffic::script{json_stream});
}

TEST_F(script_test, NoRangeInRanges)
{
    auto json = build_script();
    traffic::json_reader ranges_empty("{}", "{}");
    json.set<traffic::json_reader>("/ranges", ranges_empty);
    json_stream << json.as_string();
    ASSERT_NO_THROW(traffic::script{json_stream});
}

TEST_F(script_test, PostProcessLastMessageReturnsFalse)
{
    const auto json = build_script();
    json_stream << json.as_string();
    traffic::script script{json_stream};
    ASSERT_FALSE(script.post_process(traffic::answer_type(200, "OK")));
}

TEST_F(script_test, PostProcessTwoAnswers)
{
    auto json = build_script();
    json.set<std::vector<std::string>>("/flow", {"test1", "test1"});
    json_stream << json.as_string();
    traffic::script script{json_stream};
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
    json_stream << json.as_string();
    traffic::script script{json_stream};

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
    json_stream << json.as_string();
    traffic::script script{json_stream};

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
    json_stream << json.as_string();
    traffic::script script{json_stream};

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
    json_stream << json.as_string();
    traffic::script script{json_stream};

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
    json_stream << json.as_string();
    traffic::script script{json_stream};

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
    json_stream << json.as_string();
    traffic::script script{json_stream};

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
    json_stream << json.as_string();
    traffic::script script{json_stream};

    traffic::json_reader answer;
    answer.set<std::string>(expected_path, "I am a string");
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, answer.as_string())));
    EXPECT_STRCASEEQ(answer.as_string().c_str(), script.get_next_body().c_str());
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
    json_stream << json.as_string();
    traffic::script script{json_stream};

    traffic::json_reader answer;
    answer.set<int>(expected_path, 53);
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, answer.as_string())));
    EXPECT_STRCASEEQ(answer.as_string().c_str(), script.get_next_body().c_str());
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
    json_stream << json.as_string();
    traffic::script script{json_stream};

    traffic::json_reader answer;
    answer.set<std::string>(expected_path + "/sub_path1", "hi there");
    answer.set<int>(expected_path + "/sub_path1", 235);
    ASSERT_TRUE(script.post_process(traffic::answer_type(200, answer.as_string())));
    EXPECT_STRCASEEQ(answer.as_string().c_str(), script.get_next_body().c_str());
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
    json_stream << json.as_string();
    traffic::script script{json_stream};

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
    json_stream << json.as_string();
    traffic::script script{json_stream};

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
    json_stream << json.as_string();

    traffic::script script{json_stream};
    script.parse_ranges(std::map<std::string, int64_t>{{"my_range", 55}});
    ASSERT_STREQ("/my/url/55", script.get_next_url().c_str());
    ASSERT_STREQ("{\"data\":\"in-range-55\"}", script.get_next_body().c_str());
}
