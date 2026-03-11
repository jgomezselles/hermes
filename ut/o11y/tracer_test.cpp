#include "tracer.hpp"

#include <gtest/gtest.h>
#include <nghttp2/asio_http2.h>
#include <thread>

#include "observability.hpp"

#include "opentelemetry/trace/span.h"
#include "opentelemetry/trace/span_id.h"
#include "opentelemetry/trace/trace_id.h"

using namespace std::chrono_literals;
using nghttp2::asio_http2::header_map;
using nghttp2::asio_http2::header_value;

namespace otel_std = opentelemetry::nostd;
namespace otel_trace = opentelemetry::trace;

class tracer_test : public testing::Test
{
public:
    void TearDown() override {};

    std::string trace_str(const opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> &span)
    {
        char buf[opentelemetry::trace::TraceId::kSize * 2];  // 32 chars
        span->GetContext().trace_id().ToLowerBase16(buf);
        return std::string(buf, sizeof(buf));
    }

    std::string span_str(const opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> &span)
    {
        char buf[opentelemetry::trace::SpanId::kSize * 2];    // 16 chars
        span->GetContext().span_id().ToLowerBase16(buf);
        return std::string(buf, sizeof(buf));
    }
};

TEST_F(tracer_test, GetTracerWhenCannotConnect)
{
    o11y::init_tracer("http://holis:9090");
    o11y::get_tracer("hermes_client");
    o11y::cleanup_tracer();
}

TEST_F(tracer_test, CreateOrphanChildSpanDoesNotDump)
{
    otel_std::shared_ptr<otel_trace::Span> s;
    auto span = o11y::create_child_span("myspan", s);
}

TEST_F(tracer_test, CreateSpanWithoutInitDoesNotDump)
{
    otel_std::shared_ptr<otel_trace::Span> s;
    auto span = o11y::create_span("span1");
}

// This test is overkill but helps to understand behavior
TEST_F(tracer_test, ChildSpansHaveTheSameParent)
{
    o11y::init_tracer("");

    auto vader = o11y::create_span("Anakin");
    auto vader_trace_id = vader->GetContext().trace_id();
    auto vader_span_id = vader->GetContext().span_id();
    std::string vader_trace_str = trace_str(vader);
    std::string vader_span_str = span_str(vader);

    auto luke = o11y::create_child_span("Luke", vader);
    auto luke_trace_id = luke->GetContext().trace_id();
    auto luke_span_id = luke->GetContext().span_id();
    std::string luke_trace_str = trace_str(luke);
    std::string luke_span_str = span_str(luke);

    //Same family
    ASSERT_EQ(luke_trace_id, vader_trace_id);
    ASSERT_STREQ(luke_trace_str.c_str(), vader_trace_str.c_str());

    //Different uid
    ASSERT_NE(luke_span_id, vader_span_id);
    ASSERT_STRNE(luke_span_str.c_str(), vader_span_str.c_str());

    auto leia = o11y::create_child_span("Leia", vader);
    auto leia_trace_id = leia->GetContext().trace_id();
    auto leia_span_id = leia->GetContext().span_id();
    std::string leia_trace_str = trace_str(leia);
    std::string leia_span_str = span_str(leia);

    //Same family
    ASSERT_EQ(leia_trace_id, luke_trace_id);
    ASSERT_STREQ(leia_trace_str.c_str(), luke_trace_str.c_str());
    ASSERT_EQ(leia_trace_id, vader_trace_id);
    ASSERT_STREQ(leia_trace_str.c_str(), vader_trace_str.c_str());

    //Different uid
    ASSERT_NE(leia_span_id, vader_span_id);
    ASSERT_STRNE(leia_span_str.c_str(), vader_span_str.c_str());
    ASSERT_NE(leia_span_id, luke_span_id);
    ASSERT_NE(leia_span_str.c_str(), luke_span_str.c_str());

    auto kylo = o11y::create_child_span("Kylo", leia);
    auto kylo_trace_id = kylo->GetContext().trace_id();
    auto kylo_span_id = kylo->GetContext().span_id();
    std::string kylo_trace_str = trace_str(kylo);
    std::string kylo_span_str = span_str(kylo);

    //Same family
    ASSERT_EQ(kylo_trace_id, leia_trace_id);
    ASSERT_STREQ(kylo_trace_str.c_str(), leia_trace_str.c_str());
    ASSERT_EQ(kylo_trace_id, luke_trace_id);
    ASSERT_STREQ(kylo_trace_str.c_str(), luke_trace_str.c_str());
    ASSERT_EQ(kylo_trace_id, vader_trace_id);
    ASSERT_STREQ(kylo_trace_str.c_str(), vader_trace_str.c_str());

    //Different uid
    ASSERT_NE(kylo_span_id, vader_span_id);
    ASSERT_STRNE(kylo_span_str.c_str(), vader_span_str.c_str());

    ASSERT_NE(kylo_span_id, luke_span_id);
    ASSERT_STRNE(kylo_span_str.c_str(), luke_span_str.c_str());
    ASSERT_NE(kylo_span_id, leia_span_id);
    ASSERT_STRNE(kylo_span_str.c_str(), leia_span_str.c_str());

    //Valid trace
    ASSERT_TRUE(vader_trace_id.IsValid());
    ASSERT_TRUE(luke_trace_id.IsValid());
    ASSERT_TRUE(leia_trace_id.IsValid());
    ASSERT_TRUE(kylo_trace_id.IsValid());

    //Valid span
    ASSERT_TRUE(vader_span_id.IsValid());
    ASSERT_TRUE(luke_span_id.IsValid());
    ASSERT_TRUE(leia_span_id.IsValid());
    ASSERT_TRUE(kylo_span_id.IsValid());
}

TEST_F(tracer_test, AsyncChildSpansHaveTheSameParent)
{
    o11y::init_tracer("");

    nghttp2::asio_http2::header_map headers;
    auto vader = o11y::create_span("Anakin");
    auto vader_trace_id = vader->GetContext().trace_id();
    auto vader_span_id = vader->GetContext().span_id();

    //This ensures Chewee's family is the active one
    auto tracer = o11y::get_tracer("hermes_client");
    auto chewee = tracer->StartSpan("Chewbacca");
    auto outer_scope = tracer->WithActiveSpan(chewee);
    auto chewee_son = tracer->StartSpan("Chewbacca's son");
    outer_scope = tracer->WithActiveSpan(chewee_son);

    auto chewee_trace_id = chewee->GetContext().trace_id();
    auto chewee_son_trace_id = chewee_son->GetContext().trace_id();

    ASSERT_EQ(chewee_son_trace_id, chewee_trace_id);
    ASSERT_TRUE(chewee->GetContext().IsValid());
    ASSERT_TRUE(chewee_son->GetContext().IsValid());

    //Anakin's Offspring. Created here, so we can test results later
    ot_std::shared_ptr<ot_trace::Span> luke, leia, kylo;

    //Raised far far away from each other
    std::vector<std::thread> threads;
    threads.push_back(std::thread{
            [&vader, &luke]
            {
                luke = o11y::create_child_span("Luke", vader);
            }});

    threads.push_back(std::thread{
            [&vader, &leia]
            {
                leia = o11y::create_child_span("Leia", vader);
            }});

    for (auto& thread : threads)
    {
        thread.join();
    }

    //Simulating span context on headers that will travel
    o11y::inject_trace_context(leia, headers);

    {
        //Doing this in another scope and another thread for the fun of it,
        //to demonstrate that it works
        std::thread t{[&, this] {kylo = o11y::create_child_span_from_remote(headers, "Kylo Ren");}};
        t.join();
    }

    auto ctx = opentelemetry::context::RuntimeContext::GetCurrent();

    auto leia_trace_id = leia->GetContext().trace_id();
    auto leia_span_id = leia->GetContext().span_id();
    auto luke_trace_id = luke->GetContext().trace_id();
    auto luke_span_id = luke->GetContext().span_id();
    auto kylo_trace_id = kylo->GetContext().trace_id();
    auto kylo_span_id = kylo->GetContext().span_id();

    ASSERT_EQ(vader_trace_id, luke_trace_id);
    ASSERT_EQ(vader_trace_id, leia_trace_id);
    ASSERT_EQ(vader_trace_id, kylo_trace_id);
    ASSERT_EQ(leia_trace_id, kylo_trace_id);

    ASSERT_NE(vader_trace_id, chewee_trace_id);

    ASSERT_NE(vader_span_id, luke_span_id);
    ASSERT_NE(vader_span_id, leia_span_id);
    ASSERT_NE(vader_span_id, kylo_span_id);
}

TEST_F(tracer_test, ExtractAndInjectSpans)
{
    o11y::init_tracer("hermes_client");

    nghttp2::asio_http2::header_map headers;
    auto vader = o11y::create_span("Anakin");
    auto vader_trace_id = vader->GetContext().trace_id();
    auto vader_span_id = vader->GetContext().span_id();

    //Injecting span context on headers that will travel
    o11y::inject_trace_context(vader, headers);

    //Let's imagine we receive these headers in our server
    auto luke = o11y::create_child_span_from_remote(headers, "Luke");
    auto luke_trace_id = luke->GetContext().trace_id();
    auto luke_span_id = luke->GetContext().span_id();

    //I AM YOUR FATHER!
    EXPECT_EQ(luke_trace_id, vader_trace_id);

    //But we're different!
    EXPECT_NE(luke_span_id, vader_span_id);
}

TEST_F(tracer_test, ExtractInvalidCreatesValidSpan)
{
    o11y::init_tracer("hermes_client");

    auto vader = o11y::create_span("Anakin");
    auto vader_trace_id = vader->GetContext().trace_id();
    auto vader_span_id = vader->GetContext().span_id();


    //We receive empty headers in our server
    nghttp2::asio_http2::header_map headers;
    auto luke = o11y::create_child_span_from_remote(headers, "Luke");
    auto luke_trace_id = luke->GetContext().trace_id();
    auto luke_span_id = luke->GetContext().span_id();

    //We'll create a valid span anyways
    ASSERT_TRUE(luke->GetContext().IsValid());
    EXPECT_TRUE(trace_str(luke).size());
    EXPECT_TRUE(span_str(luke).size());

    //I AM NOT YOUR FATHER!
    EXPECT_NE(luke_trace_id, vader_trace_id);
    EXPECT_NE(luke_span_id, vader_span_id);
}

