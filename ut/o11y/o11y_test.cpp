#include <gtest/gtest.h>
#include "observability.hpp"

using namespace std::chrono_literals;

class o11y_test : public testing::Test
{
public:
    void TearDown() override
    {
        unsetenv("OTLP_METRICS_ENDPOINT");
        unsetenv("OTLP_TRACES_ENDPOINT");
    };
};

TEST_F(o11y_test, InitAndShutDownObservabilityWithoutEnvVariables)
{
    o11y::init_observability();
    o11y::shutdown_observability();
}

TEST_F(o11y_test, InitAndShutDownObservabilityWithEnvVariables)
{
    setenv("OTLP_METRICS_ENDPOINT", "http://dummy-service:8080/v1/metrics",0);
    setenv("OTLP_TRACES_ENDPOINT", "http://dummy-service:8080/v1/traces",0);
    o11y::init_observability();
    o11y::shutdown_observability();
}

