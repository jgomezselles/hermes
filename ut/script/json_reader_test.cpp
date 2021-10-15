#include "json_reader.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace traffic
{
TEST(json_reader_test, ParseWrongJson)
{
    ASSERT_THROW(json_reader("This is not a json", {}), std::logic_error);
}

TEST(json_reader_test, ParseEmptyString)
{
    ASSERT_THROW(json_reader({}, {}), std::logic_error);
}

TEST(json_reader_test, ParseEmptyStringJsonRepresentation)
{
    std::string json_str = R"("")";
    auto json = json_reader(json_str, {});
    ASSERT_EQ(json_str, json.as_string());
}

TEST(json_reader_test, ParseEmptyObject)
{
    std::string json_str = R"("{}")";
    auto json = json_reader(json_str, {});
    ASSERT_EQ(json_str, json.as_string());
}

TEST(json_reader_test, ParseEmptyArray)
{
    std::string json_str = R"("[]")";
    auto json = json_reader(json_str, {});
    ASSERT_EQ(json_str, json.as_string());
}

TEST(json_reader_test, ParseInt)
{
    std::string json_str = R"("666")";
    auto json = json_reader(json_str, {});
    ASSERT_EQ(json_str, json.as_string());
}

TEST(json_reader_test, ParseWrongSchema)
{
    std::string json_str = R"("")";
    EXPECT_THROW(
        {
            try
            {
                json_reader(json_str, "This is a wrong schema");
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), "Not valid schema.");
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, ErrorParsingAgainstSchema)
{
    std::string json_str = R"("This is a string")";
    std::string schema = R"({ "type": "integer"})";
    EXPECT_THROW(
        {
            try
            {
                json_reader(json_str, schema);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), "Input does not validate against the schema! ");
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, SchemaValidationOk)
{
    std::string json_str = R"("This is a string")";
    std::string schema = R"({ "type": "string"})";
    auto json = json_reader(json_str, schema);
    ASSERT_EQ(json_str, json.as_string());
}

TEST(json_reader_test, AssignmentAndComparisonOperators)
{
    std::string json_str = R"("This is a string")";
    auto json = json_reader(json_str, "");
    auto other_json = json;

    ASSERT_EQ(json, other_json);
    ASSERT_EQ(json_str, json.as_string());
    ASSERT_EQ(json.as_string(), other_json.as_string());
}

TEST(json_reader_test, CopyCtor)
{
    std::string json_str = R"({"attr1": ["arr1", "arr2"], "attr2": 5})";
    auto json = json_reader(json_str, "");
    json_reader other_json(json);
    ASSERT_EQ(json, other_json);
}

TEST(json_reader_test, SetGetString)
{
    auto json = json_reader();
    json.set<std::string>("/str", "hello");
    ASSERT_EQ("hello", json.get_value<std::string>("/str"));

    json.set<std::string>("/sub_json/sub_str", "world");
    ASSERT_EQ("world", json.get_value<std::string>("/sub_json/sub_str"));

    json_reader expected_json(R"({"str": "hello", "sub_json": {"sub_str": "world"}})", "");
    ASSERT_EQ(json, expected_json);
}

TEST(json_reader_test, WrongTypeGettingString)
{
    std::string path{"/no_str"};
    auto json = json_reader();
    json.set<int>(path, 2);

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] std::string str = json.get_value<std::string>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("String not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, StringNotFound)
{
    auto json = json_reader();
    std::string path{"/wrong_path"};

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] std::string str = json.get_value<std::string>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("String not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}
