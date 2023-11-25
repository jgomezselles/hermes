
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
namespace ot_std = opentelemetry::nostd;
namespace ot_metrics = opentelemetry::metrics;
namespace sdk_metrics = opentelemetry::sdk::metrics;

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

    sdk_metrics::PeriodicExportingMetricReaderOptions reader_options;
    reader_options.export_interval_millis = std::chrono::milliseconds(1000);
    reader_options.export_timeout_millis = std::chrono::milliseconds(500);

    auto reader = sdk_metrics::PeriodicExportingMetricReaderFactory::Create(std::move(exporter),
                                                                            reader_options);
    auto context = sdk_metrics::MeterContextFactory::Create();
    context->AddMetricReader(std::move(reader));

    auto u_provider = sdk_metrics::MeterProviderFactory::Create(std::move(context));
    ot_std::shared_ptr<ot_metrics::MeterProvider> provider(std::move(u_provider));
    ot_metrics::Provider::SetMeterProvider(provider);
}

void cleanup_metrics()
{
    // To prevent cancelling ongoing exports.
    ot_std::shared_ptr<ot_metrics::MeterProvider> provider =
        ot_metrics::Provider::GetMeterProvider();

    if (provider)
    {
        if (sdk_metrics::MeterProvider* d =
                dynamic_cast<sdk_metrics::MeterProvider*>(provider.get());
            d)
        {
            d->ForceFlush();
        }
    }
    ot_std::shared_ptr<ot_metrics::MeterProvider> noop(new ot_metrics::NoopMeterProvider());
    ot_metrics::Provider::SetMeterProvider(noop);
}

}  // namespace o11y