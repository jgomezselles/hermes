
#include "metrics.hpp"

#include <chrono>
#include <iostream>
#include <memory>

#include "opentelemetry/exporters/otlp/otlp_http_metric_exporter_factory.h"
#include "opentelemetry/metrics/provider.h"
#include "opentelemetry/sdk/metrics/export/periodic_exporting_metric_reader_factory.h"
#include "opentelemetry/sdk/metrics/meter.h"
#include "opentelemetry/sdk/metrics/meter_context_factory.h"
#include "opentelemetry/sdk/metrics/meter_provider.h"
#include "opentelemetry/sdk/metrics/meter_provider_factory.h"
#include "opentelemetry/sdk/metrics/push_metric_exporter.h"

using namespace std::chrono;

namespace o11y
{

void init_metrics_otlp_http(const std::string& url)
{
    opentelemetry::exporter::otlp::OtlpHttpMetricExporterOptions otlpOptions;
    otlpOptions.url = url;
    otlpOptions.content_type = opentelemetry::exporter::otlp::HttpRequestContentType::kBinary;
    otlpOptions.console_debug = true;
    auto exporter =
        opentelemetry::exporter::otlp::OtlpHttpMetricExporterFactory::Create(otlpOptions);

    opentelemetry::sdk::metrics::PeriodicExportingMetricReaderOptions reader_options;
    reader_options.export_interval_millis = std::chrono::milliseconds(1000);
    reader_options.export_timeout_millis = std::chrono::milliseconds(500);

    auto reader = opentelemetry::sdk::metrics::PeriodicExportingMetricReaderFactory::Create(
        std::move(exporter), reader_options);
    auto context = opentelemetry::sdk::metrics::MeterContextFactory::Create();
    context->AddMetricReader(std::move(reader));

    auto u_provider = opentelemetry::sdk::metrics::MeterProviderFactory::Create(std::move(context));
    opentelemetry::v1::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> provider(
        std::move(u_provider));
    opentelemetry::metrics::Provider::SetMeterProvider(provider);
}

void cleanup_metrics()
{
    opentelemetry::v1::nostd::shared_ptr<opentelemetry::metrics::MeterProvider> none;
    opentelemetry::metrics::Provider::SetMeterProvider(none);
}

}  // namespace o11y