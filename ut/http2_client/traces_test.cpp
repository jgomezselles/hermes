#include <gtest/gtest.h>

#include "opentelemetry/exporters/ostream/span_exporter_factory.h"
#include "opentelemetry/sdk/trace/exporter.h"
#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/simple_processor_factory.h"
#include "opentelemetry/sdk/trace/tracer_provider_factory.h"
#include "opentelemetry/trace/provider.h"

namespace observ
{
class traces_test : public ::testing::Test
{
public:
    void init_tracer()
    {
        auto exporter = opentelemetry::exporter::trace::OStreamSpanExporterFactory::Create();
        auto processor =
            opentelemetry::sdk::trace::SimpleSpanProcessorFactory::Create(std::move(exporter));
        std::shared_ptr<opentelemetry::trace::TracerProvider> provider =
            opentelemetry::sdk::trace::TracerProviderFactory::Create(std::move(processor));

        // Set the global trace provider
        opentelemetry::trace::Provider::SetTracerProvider(provider);
    }

    void cleanup_tracer()
    {
        std::shared_ptr<opentelemetry::trace::TracerProvider> none;
        opentelemetry::trace::Provider::SetTracerProvider(none);
    }

    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> get_tracer()
    {
        auto provider = opentelemetry::trace::Provider::GetTracerProvider();
        return provider->GetTracer("my tracer", OPENTELEMETRY_SDK_VERSION);
    }
    void SetUp() override { init_tracer(); };

    void TearDown() override { cleanup_tracer(); };
};

TEST_F(traces_test, MyFirstTest)
{
    opentelemetry::v1::nostd::shared_ptr<opentelemetry::v1::trace::Span> span1, span2, span3;
    span1 = get_tracer()->StartSpan("span1");
    auto first_scope = opentelemetry::trace::Scope(span1);
    {
        span2 = get_tracer()->StartSpan("span2");
        auto scoped_cope = opentelemetry::trace::Scope(span2);
        {
            span3 = get_tracer()->StartSpan("span3");
            auto third_span = opentelemetry::trace::Scope(span3);
        }
    }
}

}  // namespace observ
