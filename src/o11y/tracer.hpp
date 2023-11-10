#pragma once

#include <string>

namespace o11y
{

void init_tracer(const std::string& url);
void cleanup_tracer();
// opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> get_tracer(const std::string&
// tracer_name);
}  // namespace o11y