#pragma once

#include <nghttp2/asio_http2.h>

#include <string>
#include <iostream>

#include "opentelemetry/context/propagation/global_propagator.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/trace/tracer.h"

namespace ot_std = opentelemetry::nostd;
namespace ot_trace = opentelemetry::trace;

namespace o11y
{

using nghttp2::asio_http2::header_map;
using nghttp2::asio_http2::header_value;

class HttpTextMapCarrier : public opentelemetry::context::propagation::TextMapCarrier
{
public:
    HttpTextMapCarrier(nghttp2::asio_http2::header_map& headers) : headers_(headers) {}
    HttpTextMapCarrier() = default;

    virtual ot_std::string_view Get(opentelemetry::nostd::string_view key) const noexcept override
    {
        if (const auto it = headers_.find(key.data()); it != headers_.end())
        {
            return it->second.value;
        }
        return "";
    }

    virtual void Set(ot_std::string_view k, ot_std::string_view v) noexcept override
    {
        headers_.emplace(k, header_value{std::string(v), false});
    }

    nghttp2::asio_http2::header_map& headers_;
};

void init_tracer(const std::string& url);
void cleanup_tracer();

ot_std::shared_ptr<ot_trace::Tracer> get_tracer(const std::string& tracer_name);

ot_std::shared_ptr<ot_trace::Span> create_span(const std::string& name);

ot_std::shared_ptr<ot_trace::Span> create_child_span(
    const std::string& name, const ot_std::shared_ptr<ot_trace::Span>& parent);

void inject_trace_context(const ot_std::shared_ptr<ot_trace::Span>& span,
                          nghttp2::asio_http2::header_map& headers);

ot_std::shared_ptr<ot_trace::Span> create_child_span_from_remote(
    nghttp2::asio_http2::header_map& headers,
    const std::string& name);

}  // namespace o11y