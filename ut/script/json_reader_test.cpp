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
