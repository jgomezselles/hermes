#include "script.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <functional>
#include <map>
#include <sstream>

#include "action_builder.h"
#include "json_element_builder.h"
#include "json_object_builder.h"
#include "message_builder.h"
#include "range_builder.h"
#include "response_builder.h"
#include "script_builder.h"

class script_test : public ::testing::Test
{
protected:
    void buildStream(const std::string &json)
    {
        json_stream.clear();
        json_stream << json;
    }

    std::stringstream json_stream;
    ut_helpers::script_builder script_builder;
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
    buildStream(script_builder.ranges(std::nullopt).build());
    std::unique_ptr<traffic::script> script;
    EXPECT_NO_THROW(script = std::make_unique<traffic::script>(json_stream));
    EXPECT_EQ("public-dns", script->get_server_dns());
    EXPECT_EQ("8686", script->get_server_port());
    EXPECT_EQ(2000, script->get_timeout_ms());
    EXPECT_THAT(std::vector<std::string>{"test1"},
                testing::ContainerEq(script->get_message_names()));
}

TEST_F(script_test, ValidationErrorNoFlow)
{
    buildStream(script_builder.flow(std::nullopt).build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoMessages)
{
    buildStream(script_builder.messages(std::nullopt).build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoDns)
{
    buildStream(script_builder.dns(std::nullopt).build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoPort)
{
    buildStream(script_builder.port(std::nullopt).build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoTimeout)
{
    buildStream(script_builder.timeout(std::nullopt).build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoMethodInMessage)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1).method(std::nullopt).build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoUrlInMessage)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1).url(std::nullopt).build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoBodyInMessage)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1).response(std::nullopt).build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoResponseInMessage)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1).response(std::nullopt).build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoCodeInResponseInMessage)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1)
                            .response(ut_helpers::response_builder(std::nullopt).build())
                            .build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, TotalMessageIsReserved)
{
    buildStream(
        script_builder.flow(std::vector<std::string>{"Total"})
            .messages(std::vector<std::string>{ut_helpers::message_builder("Total").build()})
            .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageInFlowNotFound)
{
    buildStream(script_builder.flow(std::vector<std::string>{"\"test1\"", "\"test2\""}).build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithSFA)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1)
                                                           .sfa(ut_helpers::action_builder()
                                                                    .name("meta")
                                                                    .path("/$meta")
                                                                    .value_type("object")
                                                                    .build())
                                                           .build()})
                    .build());
    ASSERT_NO_THROW(traffic::script{json_stream});
}

TEST_F(script_test, ValidationErrorMessageWithSFANoName)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1)
                                                           .sfa(ut_helpers::action_builder()
                                                                    .name(std::nullopt)
                                                                    .path("/$meta")
                                                                    .value_type("object")
                                                                    .build())
                                                           .build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorMessageWithSFANoPath)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1)
                                                           .sfa(ut_helpers::action_builder()
                                                                    .name("meta")
                                                                    .path(std::nullopt)
                                                                    .value_type("object")
                                                                    .build())
                                                           .build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorMessageWithSFANoValueType)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1)
                                                           .sfa(ut_helpers::action_builder()
                                                                    .name("meta")
                                                                    .path("/$meta")
                                                                    .value_type(std::nullopt)
                                                                    .build())
                                                           .build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, MessageWithATB)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1)
                                                           .atb(ut_helpers::action_builder()
                                                                    .name("meta")
                                                                    .path("/$meta")
                                                                    .value_type("object")
                                                                    .build())
                                                           .build()})
                    .build());
    ASSERT_NO_THROW(traffic::script{json_stream});
}

TEST_F(script_test, ValidationErrorMessageWithATBNoName)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1)
                                                           .atb(ut_helpers::action_builder()
                                                                    .name(std::nullopt)
                                                                    .path("/$meta")
                                                                    .value_type("object")
                                                                    .build())
                                                           .build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorMessageWithATBNoPath)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1)
                                                           .atb(ut_helpers::action_builder()
                                                                    .name("meta")
                                                                    .path(std::nullopt)
                                                                    .value_type("object")
                                                                    .build())
                                                           .build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorMessageWithATBNoValueType)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1)
                                                           .atb(ut_helpers::action_builder()
                                                                    .name("meta")
                                                                    .path("/$meta")
                                                                    .value_type(std::nullopt)
                                                                    .build())
                                                           .build()})
                    .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoMinInRange)
{
    buildStream(
        script_builder
            .ranges(std::vector<std::string>{ut_helpers::range_builder().min(std::nullopt).build()})
            .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoMaxInRange)
{
    buildStream(
        script_builder
            .ranges(std::vector<std::string>{ut_helpers::range_builder().max(std::nullopt).build()})
            .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorMaxUnderMinInRange)
{
    buildStream(
        script_builder
            .ranges(std::vector<std::string>{ut_helpers::range_builder().min(10).max(5).build()})
            .build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, ValidationErrorNoRangeInRanges)
{
    buildStream(script_builder.ranges(std::vector<std::string>{}).build());
    ASSERT_THROW(traffic::script{json_stream}, std::logic_error);
}

TEST_F(script_test, PostProcessIsLast)
{
    buildStream(script_builder.build());
    traffic::script script{json_stream};
    ASSERT_FALSE(script.post_process(traffic::answer_type(200, "OK")));
}

TEST_F(script_test, PostProcessIsOK)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{ut_helpers::message_builder(1).build(),
                                                       ut_helpers::message_builder(2).build()})
                    .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
                    .build());
    traffic::script script{json_stream};
    EXPECT_TRUE(script.post_process(traffic::answer_type(200, "OK")));
    EXPECT_FALSE(script.post_process(traffic::answer_type(200, "OK")));
}

TEST_F(script_test, PostProcessCorrectStringValueINSFA)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1)
                            .method("POST")
                            .sfa(ut_helpers::action_builder()
                                     .name("meta")
                                     .path("/url")
                                     .value_type("string")
                                     .build())
                            .build(),
                        ut_helpers::message_builder(2).method("POST").build()})
                    .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
                    .build());
    traffic::script script{json_stream};
    auto answer = ut_helpers::json_object_builder().manipulate_string("url", "OK").build();
    EXPECT_TRUE(script.post_process(traffic::answer_type(200, answer)));
}

TEST_F(script_test, PostProcessCorrectIntValueINSFA)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1)
                            .method("POST")
                            .sfa(ut_helpers::action_builder()
                                     .name("meta")
                                     .path("/port")
                                     .value_type("int")
                                     .build())
                            .build(),
                        ut_helpers::message_builder(2).method("POST").build()})
                    .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
                    .build());
    traffic::script script{json_stream};
    auto answer = ut_helpers::json_object_builder().manipulate_int("port", 9090).build();
    EXPECT_TRUE(script.post_process(traffic::answer_type(200, answer)));
}

TEST_F(script_test, PostProcessCorrectObjectValueINSFA)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1)
                            .method("POST")
                            .sfa(ut_helpers::action_builder()
                                     .name("meta")
                                     .path("/data")
                                     .value_type("object")
                                     .build())
                            .build(),
                        ut_helpers::message_builder(2).method("POST").build()})
                    .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
                    .build());
    traffic::script script{json_stream};
    auto answer =
        ut_helpers::json_object_builder()
            .manipulate_object(
                "data", ut_helpers::json_object_builder().manipulate_int("port", 9090).build())
            .build();
    EXPECT_TRUE(script.post_process(traffic::answer_type(200, answer)));
}

TEST_F(script_test, PostProcessInCorrectStringValueINSFA)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1)
                            .method("POST")
                            .sfa(ut_helpers::action_builder()
                                     .name("meta")
                                     .path("/url")
                                     .value_type("string")
                                     .build())
                            .build(),
                        ut_helpers::message_builder(2).method("POST").build()})
                    .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
                    .build());
    traffic::script script{json_stream};
    auto answer = ut_helpers::json_object_builder().manipulate_int("url", 9090).build();
    EXPECT_FALSE(script.post_process(traffic::answer_type(200, answer)));
}

TEST_F(script_test, PostProcessInCorrectIntValueINSFA)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1)
                            .method("POST")
                            .sfa(ut_helpers::action_builder()
                                     .name("meta")
                                     .path("/port")
                                     .value_type("int")
                                     .build())
                            .build(),
                        ut_helpers::message_builder(2).method("POST").build()})
                    .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
                    .build());
    traffic::script script{json_stream};
    auto answer = ut_helpers::json_object_builder().manipulate_string("port", "9090").build();
    EXPECT_FALSE(script.post_process(traffic::answer_type(200, answer)));
}

TEST_F(script_test, PostProcessInCorrectObjectValueINSFA)
{
    buildStream(script_builder
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1)
                            .method("POST")
                            .sfa(ut_helpers::action_builder()
                                     .name("meta")
                                     .path("/data")
                                     .value_type("object")
                                     .build())
                            .build(),
                        ut_helpers::message_builder(2).method("POST").build()})
                    .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
                    .build());
    traffic::script script{json_stream};
    auto answer = ut_helpers::json_object_builder().manipulate_int("data", 9090).build();
    EXPECT_FALSE(script.post_process(traffic::answer_type(200, answer)));
}

TEST_F(script_test, PostProcessCorrectStringValueINSFAUsedInATB)
{
    buildStream(
        script_builder
            .messages(std::vector<std::string>{
                ut_helpers::message_builder(1)
                    .method("POST")
                    .sfa(ut_helpers::action_builder()
                             .name("meta")
                             .path("/url")
                             .value_type("string")
                             .build())
                    .build(),
                ut_helpers::message_builder(2)
                    .method("POST")
                    .body(ut_helpers::json_object_builder().manipulate_string("url", "").build())
                    .atb(ut_helpers::action_builder()
                             .name("meta")
                             .path("/url")
                             .value_type("string")
                             .build())
                    .build()})
            .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
            .build());
    traffic::script script{json_stream};
    auto answer = ut_helpers::json_object_builder().manipulate_string("url", "OK").build();
    EXPECT_TRUE(script.post_process(traffic::answer_type(200, answer)));
    EXPECT_STRCASEEQ(answer.c_str(), script.get_next_body().c_str());
}

TEST_F(script_test, PostProcessCorrectIntValueINSFAUsedInATB)
{
    buildStream(
        script_builder
            .messages(std::vector<std::string>{
                ut_helpers::message_builder(1)
                    .method("POST")
                    .sfa(ut_helpers::action_builder()
                             .name("meta")
                             .path("/port")
                             .value_type("int")
                             .build())
                    .build(),
                ut_helpers::message_builder(2)
                    .method("POST")
                    .body(ut_helpers::json_object_builder().manipulate_int("port", 0).build())
                    .atb(ut_helpers::action_builder()
                             .name("meta")
                             .path("/port")
                             .value_type("int")
                             .build())
                    .build()})
            .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
            .build());
    traffic::script script{json_stream};
    auto answer = ut_helpers::json_object_builder().manipulate_int("port", 9090).build();
    EXPECT_TRUE(script.post_process(traffic::answer_type(200, answer)));
    EXPECT_STRCASEEQ(answer.c_str(), script.get_next_body().c_str());
}

TEST_F(script_test, PostProcessCorrectObjectValueINSFAUsedInATB)
{
    buildStream(
        script_builder
            .messages(std::vector<std::string>{
                ut_helpers::message_builder(1)
                    .method("POST")
                    .sfa(ut_helpers::action_builder()
                             .name("meta")
                             .path("/data")
                             .value_type("object")
                             .build())
                    .build(),
                ut_helpers::message_builder(2)
                    .method("POST")
                    .body(ut_helpers::json_object_builder().manipulate_object("data", "{}").build())
                    .atb(ut_helpers::action_builder()
                             .name("meta")
                             .path("/data")
                             .value_type("object")
                             .build())
                    .build()})
            .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
            .build());
    traffic::script script{json_stream};
    auto payload = ut_helpers::json_object_builder().manipulate_int("port", 9090).build();
    auto answer = ut_helpers::json_object_builder().manipulate_object("data", payload).build();
    EXPECT_TRUE(script.post_process(traffic::answer_type(200, answer)));
    EXPECT_STRCASEEQ(answer.c_str(), script.get_next_body().c_str());
}

TEST_F(script_test, PostProcessNotFoundValueINSFAToUseInATB)
{
    buildStream(
        script_builder
            .messages(std::vector<std::string>{
                ut_helpers::message_builder(1).method("POST").build(),
                ut_helpers::message_builder(2)
                    .method("POST")
                    .body(ut_helpers::json_object_builder().manipulate_string("url", "").build())
                    .atb(ut_helpers::action_builder()
                             .name("meta")
                             .path("/url")
                             .value_type("string")
                             .build())
                    .build()})
            .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
            .build());
    traffic::script script{json_stream};
    auto answer = ut_helpers::json_object_builder().manipulate_string("url", "OK").build();
    EXPECT_FALSE(script.post_process(traffic::answer_type(200, answer)));
}

TEST_F(script_test, PostProcessInCorrectTypeValueINSFAUsedInATB)
{
    buildStream(
        script_builder
            .messages(std::vector<std::string>{
                ut_helpers::message_builder(1)
                    .method("POST")
                    .sfa(ut_helpers::action_builder()
                             .name("meta")
                             .path("/url")
                             .value_type("string")
                             .build())
                    .build(),
                ut_helpers::message_builder(2)
                    .method("POST")
                    .body(ut_helpers::json_object_builder().manipulate_string("url", "").build())
                    .atb(ut_helpers::action_builder()
                             .name("meta")
                             .path("/url")
                             .value_type("int")
                             .build())
                    .build()})
            .flow(std::vector<std::string>{"\"test1\"", "\"test2\""})
            .build());
    traffic::script script{json_stream};
    auto answer = ut_helpers::json_object_builder().manipulate_string("url", "OK").build();
    EXPECT_FALSE(script.post_process(traffic::answer_type(200, answer)));
}

TEST_F(script_test, ParseRangesInRangeValue)
{
    buildStream(script_builder
                    .ranges(std::vector<std::string>{
                        ut_helpers::range_builder("my_range").min(50).max(60).build()})
                    .messages(std::vector<std::string>{
                        ut_helpers::message_builder(1)
                            .method("POST")
                            .url("/my/url/<my_range>")
                            .body(ut_helpers::json_object_builder()
                                      .manipulate_string("data", "in-range-<my_range>")
                                      .build())
                            .build()})
                    .build());
    traffic::script script{json_stream};
    script.parse_ranges(std::map<std::string, int64_t>{{"my_range", 55}});
    EXPECT_STREQ("/my/url/55", script.get_next_url().c_str());
    EXPECT_STREQ("{\"data\":\"in-range-55\"}", script.get_next_body().c_str());
}