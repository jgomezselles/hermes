#include "tracer.hpp"

#include <gtest/gtest.h>

#include "observability.hpp"

using namespace std::chrono_literals;

namespace otel_std = opentelemetry::nostd;
namespace otel_trace = opentelemetry::trace;

class tracer_test : public testing::Test
{
public:
    void TearDown() override{};
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