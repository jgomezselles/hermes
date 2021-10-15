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

TEST(json_reader_test, SetGetBool)
{
    auto json = json_reader();
    json.set<bool>("/bull", true);
    ASSERT_EQ(true, json.get_value<bool>("/bull"));

    json.set<bool>("/sub_json/sub_bool", false);
    ASSERT_EQ(false, json.get_value<bool>("/sub_json/sub_bool"));

    json_reader expected_json(R"({"bull": true, "sub_json": {"sub_bool": false}})", "");
    ASSERT_EQ(json, expected_json);
}

TEST(json_reader_test, WrongTypeGettingBool)
{
    std::string path{"/no_bool"};
    auto json = json_reader();
    json.set<int>(path, 2);

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] bool b = json.get_value<bool>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Bool not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);

}

TEST(json_reader_test, WrongTypeGettingBoolZero)
{
    std::string path{"/no_bool"};
    auto json = json_reader();
    json.set<int>(path, 0);

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] bool b = json.get_value<bool>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Bool not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, WrongTypeGettingBoolOne)
{
    std::string path{"/no_bool"};
    auto json = json_reader();
    json.set<int>(path, 1);

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] bool b = json.get_value<bool>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Bool not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, BoolNotFound)
{
    auto json = json_reader();
    std::string path{"/wrong_path"};

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] bool b = json.get_value<bool>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Bool not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, SetGetInt)
{
    auto json = json_reader();
    json.set<int>("/int", 1);
    ASSERT_EQ(1, json.get_value<int>("/int"));

    json.set<int>("/sub_json/sub_int", 5);
    ASSERT_EQ(5, json.get_value<int>("/sub_json/sub_int"));

    json_reader expected_json(R"({"int": 1, "sub_json": {"sub_int": 5}})", "");
    ASSERT_EQ(json, expected_json);
}

TEST(json_reader_test, WrongTypeGettingInt)
{
    std::string path{"/no_int"};
    auto json = json_reader();
    json.set<std::string>(path, "I'm a string");

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] int i = json.get_value<int>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Integer not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);

}

TEST(json_reader_test, WrongTypeGettingIntZero)
{
    std::string path{"/no_int"};
    auto json = json_reader();
    json.set<bool>(path, false);

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] int i = json.get_value<int>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Integer not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, WrongTypeGettingIntOne)
{
    std::string path{"/no_int"};
    auto json = json_reader();
    json.set<bool>(path, true);

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] int i = json.get_value<int>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Integer not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, IntNotFound)
{
    auto json = json_reader();
    std::string path{"/wrong_path"};

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] int i = json.get_value<int>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Integer not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, SetGetObject)
{
    auto json = json_reader();
    json_reader sub_json(R"({"sub_json": {"sub_json_int": 5}})", "");
    json.set<json_reader>("/json", sub_json);
    ASSERT_EQ(sub_json, json.get_value<json_reader>("/json"));
    ASSERT_EQ(5, json.get_value<int>("/json/sub_json/sub_json_int"));

    json_reader expected_json(R"({"json": {"sub_json": {"sub_json_int": 5}}})", "");
    ASSERT_EQ(json, expected_json);
}

TEST(json_reader_test, WrongTypeGettingObject)
{
    std::string path{"/no_object"};
    auto json = json_reader();
    json.set<std::string>(path, "I'm a string");

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] json_reader j = json.get_value<json_reader>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Object not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);

}

TEST(json_reader_test, ObjectNotFound)
{
    auto json = json_reader();
    std::string path{"/wrong_path"};

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] json_reader j = json.get_value<json_reader>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Object not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}

using str_array = std::vector<std::string>;

TEST(json_reader_test, SetGetStrArray)
{
    auto json = json_reader();
    str_array array{"first", "second"};
    json.set<str_array>("/array", array);
    ASSERT_EQ(array, json.get_value<str_array>("/array"));

    json_reader expected_json(R"({"array": ["first", "second"]})", "");
    ASSERT_EQ(json, expected_json);
}

TEST(json_reader_test, EmptyStrArray)
{
    auto json = json_reader();
    str_array array{};
    json.set<str_array>("/array", array);
    ASSERT_EQ(array, json.get_value<str_array>("/array"));

    json_reader expected_json(R"({"array": []})", "");
    ASSERT_EQ(json, expected_json);
}

TEST(json_reader_test, WrongTypeGettingStrArray)
{
    std::string path{"/no_array"};
    auto json = json_reader();
    json.set<std::string>(path, "I'm a string");

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] str_array sa = json.get_value<str_array>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Array not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);

}

TEST(json_reader_test, WrongTypeElementInStrArray)
{
    std::string path{"/array_of_ints"};
    auto json = json_reader(R"( { "array_of_ints": [1,2,3] } )", "");

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] str_array sa = json.get_value<str_array>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Expected string but found other in array under " + path).c_str());
                throw;
            }
        },
        std::logic_error);

}

TEST(json_reader_test, GetStrArrayNotFound)
{
    auto json = json_reader();
    std::string path{"/wrong_path"};

    EXPECT_THROW(
        {
            try
            {
                [[maybe_unused]] str_array sa = json.get_value<str_array>(path);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), std::string("Array not found in " + path).c_str());
                throw;
            }
        },
        std::logic_error);
}

TEST(json_reader_test, GetAttributesNoObject)
{
    auto json = json_reader(R"("This is a string, not an object")", "");
    auto attrs = json.get_attributes();
    ASSERT_TRUE(attrs.empty());
}

TEST(json_reader_test, GetAttributesEmptyObject)
{
    auto json = json_reader(R"({ })", "");
    auto attrs = json.get_attributes();
    ASSERT_TRUE(attrs.empty());
}

TEST(json_reader_test, GetAttributes)
{
    auto json = json_reader(R"({ "attr1": {}, "attr2": "stringy", "attr3": 3, "attr4": false, "attr5": [] })", "");
    auto attrs = json.get_attributes();
    std::vector<std::string> expected_attrs{"attr1", "attr2", "attr3", "attr4", "attr5"};
    ASSERT_EQ(attrs, expected_attrs);
}

