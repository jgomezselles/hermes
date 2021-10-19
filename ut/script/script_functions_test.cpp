#include "script_functions.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nghttp2/asio_http2.h>

namespace traffic
{
using str_map = std::map<std::string, std::string>;

TEST(script_functions_test, CheckRepeated)
{
    std::set<std::string> unique_ids{"exists"};
    std::map<std::string, int> no_throw{{"does not exist", 9}};
    std::map<std::string, int> throws{{"exists", 9}};

    ASSERT_NO_THROW(check_repeated(unique_ids, no_throw));

    EXPECT_THROW(
        {
            try
            {
                check_repeated(unique_ids, throws);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(),
                             "exists found as repeated variable. Please, choose a different name.");
                throw;
            }
        },
        std::logic_error);
}

TEST(script_functions_test, SaveBodyFieldsString)
{
    const std::string resp_string{R"("Just a string")"}, id{"the_string"};
    str_map vars{}, fields2save{{id, ""}};

    save_body_fields(fields2save, resp_string, vars);

    const auto& it = vars.find(id);
    ASSERT_NE(it, vars.end());
    ASSERT_EQ(it->second, resp_string);
}

TEST(script_functions_test, SaveBodyFieldsInt)
{
    const std::string resp_int{"578"}, id{"the_int"};
    str_map vars{}, fields2save{{id, ""}};

    save_body_fields(fields2save, resp_int, vars);

    const auto& it = vars.find(id);
    ASSERT_NE(it, vars.end());
    ASSERT_EQ(it->second, resp_int);
}

TEST(script_functions_test, SaveBodyFieldsArray)
{
    const std::string resp_array{R"(["str1","str2",666])"}, id{"the_array"};
    str_map vars{}, fields2save{{id, ""}};

    save_body_fields(fields2save, resp_array, vars);

    const auto& it = vars.find(id);
    ASSERT_NE(it, vars.end());
    ASSERT_EQ(it->second, resp_array);
}

TEST(script_functions_test, SaveBodyFieldsStringOverwrite)
{
    const std::string resp_string{R"("Just a string")"}, id{"the_string"};
    str_map vars{{id, "Other stuff"}}, fields2save{{id, ""}};

    save_body_fields(fields2save, resp_string, vars);

    const auto& it = vars.find(id);
    ASSERT_NE(it, vars.end());
    ASSERT_EQ(it->second, resp_string);
}

TEST(script_functions_test, SaveBodyFieldsJson)
{
    const std::string resp_json{
        R"({"object1":{"field1":"str","field2":76,"field3":true,"field4":["str"]}})"};
    str_map vars{}, fields2save{
                        {"the_json", ""},
                        {"object1", "/object1"},
                        {"field1", "/object1/field1"},
                        {"field2", "/object1/field2"},
                        {"field3", "/object1/field3"},
                        {"field4", "/object1/field4"},
                    };

    save_body_fields(fields2save, resp_json, vars);

    str_map expected_vars{
        {"the_json", resp_json},
        {"object1", R"({"field1":"str","field2":76,"field3":true,"field4":["str"]})"},
        {"field1", R"("str")"},
        {"field2", "76"},
        {"field3", "true"},
        {"field4", R"(["str"])"},
    };

    ASSERT_EQ(vars, expected_vars);
}

TEST(script_functions_test, SaveHeadersNotFound)
{
    str_map vars, to_save{{"my_heather", "x-header-field"}};
    nghttp2::asio_http2::header_map empty;
    EXPECT_THROW(
        {
            try
            {
                save_headers(to_save, empty, vars);
            }
            catch (const std::logic_error& e)
            {
                EXPECT_STREQ(e.what(), "Header x-header-field not found.");
                throw;
            }
        },
        std::logic_error);
}

TEST(script_functions_test, SaveHeadersFound)
{
    str_map vars, to_save{{"my_heather", "x-header-field"}, {"other_heather", "x-other-field"}};

    nghttp2::asio_http2::header_map resp_headers{{"x-header-field", {"4567", false}},
                                                 {"x-other-field", {"hello", false}}};

    save_headers(to_save, resp_headers, vars);

    str_map expected_vars{{"my_heather", "4567"}, {"other_heather", "hello"}};

    ASSERT_EQ(vars, expected_vars);
}

TEST(script_functions_test, SaveHeadersOverwrite)
{
    str_map vars{{"my_heather", "to_overwrite"}};
    str_map to_save{{"my_heather", "x-header-field"}, {"other_heather", "x-other-field"}};

    nghttp2::asio_http2::header_map resp_headers{{"x-header-field", {"4567", false}},
                                                 {"x-other-field", {"hello", false}}};

    save_headers(to_save, resp_headers, vars);

    str_map expected_vars{{"my_heather", "4567"}, {"other_heather", "hello"}};

    ASSERT_EQ(vars, expected_vars);
}

}  // namespace traffic