#include "tracer.hpp"

#include "opentelemetry/exporters/otlp/otlp_http_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_exporter_options.h"
#include "opentelemetry/sdk/trace/batch_span_processor_factory.h"
#include "opentelemetry/sdk/trace/batch_span_processor_options.h"
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"

namespace o11y
{

void init_tracer(const std::string& url)
{
    opentelemetry::exporter::otlp::OtlpHttpExporterOptions http_opts;
    http_opts.url = url;
    auto exporter = opentelemetry::exporter::otlp::OtlpHttpExporterFactory::Create(http_opts);

    opentelemetry::sdk::trace::BatchSpanProcessorOptions opts;
    opts.max_queue_size = 2048;
    opts.max_export_batch_size = 512;
    auto processor =
        opentelemetry::sdk::trace::BatchSpanProcessorFactory::Create(std::move(exporter), opts);

    auto provider = opentelemetry::sdk::trace::TracerProviderFactory::Create(std::move(processor));
    opentelemetry::trace::Provider::SetTracerProvider(std::move(provider));
}

void cleanup_tracer()
{
    // We call ForceFlush to prevent to cancel running exportings, It's optional.
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> provider =
        opentelemetry::trace::Provider::GetTracerProvider();

    if (provider)
    {
        static_cast<opentelemetry::sdk::trace::TracerProvider*>(provider.get())->ForceFlush();
    }

    std::shared_ptr<opentelemetry::trace::TracerProvider> none;
    opentelemetry::trace::Provider::SetTracerProvider(none);
}

/*
opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> get_tracer(const std::string&
tracer_name)
{
    auto provider = opentelemetry::trace::Provider::GetTracerProvider();
    return provider->GetTracer(tracer_name);
}
*/

}  // namespace o11y