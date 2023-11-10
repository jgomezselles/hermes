#pragma once

#include <string>

namespace o11y
{

void init_metrics_otlp_http(const std::string& url);
void cleanup_metrics();

}  // namespace o11y