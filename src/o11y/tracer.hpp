#pragma once

#include <string>

#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/trace/tracer.h"

namespace ot_std = opentelemetry::nostd;
namespace ot_trace = opentelemetry::trace;

namespace o11y
{

void init_tracer(const std::string& url);
void cleanup_tracer();

ot_std::shared_ptr<ot_trace::Tracer> get_tracer(const std::string& tracer_name);

ot_std::shared_ptr<ot_trace::Span> create_span(const std::string& name);

}  // namespace o11y