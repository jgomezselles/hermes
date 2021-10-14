#include "script_reader.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>

namespace traffic
{
class script_reader_test : public ::testing::Test
{
public:
    json_reader build_script()
    {
        json_reader json;
        json.set<std::string>("/dns", "public-dns");
        json.set<std::string>("/port", "8686");
        json.set<int>("/timeout", 2000);
        json.set<std::vector<std::string>>("/flow", {"test1"});
        json.set<std::string>("/messages/test1/url", "v1/test");
        json.set<json_reader>("/messages/test1/body", {"{}", ""});
        json.set<std::string>("/messages/test1/method", "GET");
        json.set<int>("/messages/test1/response/code", 200);
        return json;
    }
};

TEST_F(script_reader_test, EmptyScript)
{
    ASSERT_THROW(script_reader(""), std::logic_error);
}

TEST_F(script_reader_test, NullScript)
{
    ASSERT_THROW(script_reader(json_reader("{}", "{}").as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoFlow)
{
    auto json = build_script();
    json.erase("/flow");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoMessages)
{
    auto json = build_script();
    json.erase("/messages");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoDns)
{
    auto json = build_script();
    json.erase("/dns");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoPort)
{
    auto json = build_script();
    json.erase("/port");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoTimeout)
{
    auto json = build_script();
    json.erase("/timeout");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoMethodInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/method");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoUrlInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/url");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoBodyInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/body");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoResponseInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/response");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, ValidationErrorNoCodeInResponseInMessage)
{
    auto json = build_script();
    json.erase("/messages/test1/response/code");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, TotalMessageIsNotErrorIfNotCalledInFlow)
{
    auto json = build_script();
    auto other_msg = json.get_value<json_reader>("/messages/test1");
    json.set<json_reader>("/messages/Total", other_msg);
    auto sr = script_reader(json.as_string());
    ASSERT_NO_THROW(sr.build_messages());
}

TEST_F(script_reader_test, TotalMessageIsReserved)
{
    auto json = build_script();
    auto other_msg = json.get_value<json_reader>("/messages/test1");
    json.set<std::vector<std::string>>("/flow", {"Total", "test1"});
    json.set<json_reader>("/messages/Total", other_msg);
    auto sr = script_reader(json.as_string());
    ASSERT_THROW(sr.build_messages(), std::logic_error);
}

TEST_F(script_reader_test, MessageInFlowNotFound)
{
    auto json = build_script();
    json.set<std::vector<std::string>>("/flow", {"test1", "test2"});
    auto sr = script_reader(json.as_string());
    ASSERT_THROW(sr.build_messages(), std::logic_error);
}

TEST_F(script_reader_test, MessageWithSFATypes)
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

    auto sr = script_reader(json.as_string());
    ASSERT_NO_THROW(sr.build_messages());
}

TEST_F(script_reader_test, MessageWithSFAWrongType)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_sfa");
    json.set<std::string>("/messages/test1/save_from_answer/path", "/some/path");
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "bool");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, MessageWithSFANoName)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/path", "/some/path");
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "string");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, MessageWithSFANoPath)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_sfa");
    json.set<std::string>("/messages/test1/save_from_answer/value_type", "object");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, MessageWithSFANoValueType)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/save_from_answer/name", "my_sfa");
    json.set<std::string>("/messages/test1/save_from_answer/path", "/some/path");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, MessageWithAFSTBTypes)
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

    auto sr = script_reader(json.as_string());
    ASSERT_NO_THROW(sr.build_messages());
}

TEST_F(script_reader_test, MessageWithAFSTBWrongType)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_sfa");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", "/some/path");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "bool");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, MessageWithAFSTBNoName)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", "/some/path");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "string");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, MessageWithAFSTBNoPath)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_sfa");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/value_type", "object");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, MessageWithAFSTBNoValueType)
{
    auto json = build_script();
    json.set<std::string>("/messages/test1/add_from_saved_to_body/name", "my_sfa");
    json.set<std::string>("/messages/test1/add_from_saved_to_body/path", "/some/path");
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, BuildRanges)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/min", 0);
    json.set<int>("/ranges/range1/max", 1000);
    json.set<int>("/ranges/range2/min", 1);
    json.set<int>("/ranges/range2/max", 30);
    auto sr = script_reader(json.as_string());
    ASSERT_NO_THROW(sr.build_ranges());
}

TEST_F(script_reader_test, NoMinInRange)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/max", 2);
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, NoMaxInRange)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/min", 2);
    ASSERT_THROW(script_reader(json.as_string()), std::logic_error);
}

TEST_F(script_reader_test, MaxLowerThanMinInRange)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/max", 2);
    json.set<int>("/ranges/range1/min", 3);
    auto sr = script_reader(json.as_string());
    ASSERT_THROW(sr.build_ranges(), std::logic_error);
}

TEST_F(script_reader_test, MaxEqualToMinInRange)
{
    auto json = build_script();
    json.set<int>("/ranges/range1/max", 666);
    json.set<int>("/ranges/range1/min", 666);
    ASSERT_NO_THROW(script_reader(json.as_string()));
}

TEST_F(script_reader_test, NoRangeInRanges)
{
    auto json = build_script();
    json_reader ranges_empty("{}", "{}");
    json.set<json_reader>("/ranges", ranges_empty);
    auto sr = script_reader(json.as_string());
    ASSERT_NO_THROW(sr.build_ranges());
}

}  // namespace traffic
