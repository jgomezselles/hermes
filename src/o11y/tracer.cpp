#include "tracer.hpp"

#include <iostream>

#include "opentelemetry/exporters/otlp/otlp_http_exporter_factory.h"
#include "opentelemetry/exporters/otlp/otlp_http_exporter_options.h"
#include "opentelemetry/sdk/trace/batch_span_processor_factory.h"
#include "opentelemetry/sdk/trace/batch_span_processor_options.h"
#include "opentelemetry/sdk/trace/provider.h"
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"

namespace o11y
{

namespace ot_std = opentelemetry::nostd;
namespace ot_trace = opentelemetry::trace;
namespace sdk_trace = opentelemetry::sdk::trace;

void init_tracer(const std::string& url)
{
    auto resource_attributes =
        opentelemetry::sdk::resource::ResourceAttributes{{"service.name", "hermes"}};
    auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attributes);

    opentelemetry::exporter::otlp::OtlpHttpExporterOptions http_opts;
    http_opts.url = url;
    auto exporter = opentelemetry::exporter::otlp::OtlpHttpExporterFactory::Create(http_opts);

    sdk_trace::BatchSpanProcessorOptions opts;
    opts.max_queue_size = 2048;
    opts.max_export_batch_size = 512;
    auto processor = sdk_trace::BatchSpanProcessorFactory::Create(std::move(exporter), opts);

    std::shared_ptr<opentelemetry::trace::TracerProvider> provider = sdk_trace::TracerProviderFactory::Create(std::move(processor), resource);

    sdk_trace::Provider::SetTracerProvider(provider);
}

void cleanup_tracer()
{
    // To prevent cancelling ongoing exports.
    ot_std::shared_ptr<ot_trace::TracerProvider> provider = ot_trace::Provider::GetTracerProvider();

    if (provider)
    {
        if (sdk_trace::TracerProvider* d = dynamic_cast<sdk_trace::TracerProvider*>(provider.get());
            d)
        {
            d->ForceFlush();
        }
    }
    ot_std::shared_ptr<ot_trace::TracerProvider> noop(new ot_trace::NoopTracerProvider());
    ot_trace::Provider::SetTracerProvider(noop);
}

ot_std::shared_ptr<ot_trace::Tracer> get_tracer(const std::string& tracer_name)
{
    auto provider = ot_trace::Provider::GetTracerProvider();
    return provider->GetTracer(tracer_name);
}

ot_std::shared_ptr<ot_trace::Span> create_span(const std::string& name)
{
    ot_trace::StartSpanOptions opts;
    opts.kind = ot_trace::SpanKind::kClient;

    auto span = get_tracer("hermes_client")->StartSpan(name, opts);
    return span;
}

ot_std::shared_ptr<ot_trace::Span> create_child_span(
    const std::string& name, const ot_std::shared_ptr<ot_trace::Span>& parent)
{
    ot_trace::StartSpanOptions opts;
    opts.kind = ot_trace::SpanKind::kClient;
    if (parent)
    {
        opts.parent = parent->GetContext();
    }

    auto span = get_tracer("hermes_client")->StartSpan(name, opts);
    return span;
}

}  // namespace o11y