#include "observability.hpp"

#include <iostream>
#include <string>

#include "metrics.hpp"
#include "tracer.hpp"

namespace
{

void init_metrics()
{
    if (const char* otlp_metrics_endpoint = std::getenv("OTLP_METRICS_ENDPOINT");
        otlp_metrics_endpoint && !std::string(otlp_metrics_endpoint).empty())
    {
        std::cerr << "Starting OTLP metrics exporter towards: " << otlp_metrics_endpoint
                  << std::endl;
        o11y::init_metrics_otlp_http(otlp_metrics_endpoint);
    }
    else
    {
        std::cerr << "OTLP_METRICS_ENDPOINT not found. Metrics won't be pushed." << std::endl;
    }
}

void init_traces()
{
    if (const char* otlp_traces_endpoint = std::getenv("OTLP_TRACES_ENDPOINT");
        otlp_traces_endpoint && !std::string(otlp_traces_endpoint).empty())
    {
        std::cerr << "Starting OTLP tracing exporter towards: " << otlp_traces_endpoint
                  << std::endl;
        o11y::init_tracer(otlp_traces_endpoint);
    }
    else
    {
        std::cerr << "OTLP_TRACES_ENDPOINT not found. Traces won't be pushed." << std::endl;
    }
}
}  // namespace

namespace o11y
{

void init_observability()
{
    init_metrics();
    init_traces();
}

void shutdown_observability()
{
    cleanup_metrics();
    cleanup_tracer();
}

}  // namespace o11y
