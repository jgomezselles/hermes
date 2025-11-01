#include <iostream>
#include <utility>
#include <vector>

#include "json_reader.hpp"
#include "opentelemetry/nostd/shared_ptr.h"
#include "opentelemetry/trace/tracer.h"
#include "script_structs.hpp"

#pragma once

namespace otel_std = opentelemetry::nostd;
namespace otel_trace = opentelemetry::trace;

namespace traffic
{
class script
{
public:
    script() = delete;
    explicit script(const std::string& path);
    explicit script(const json_reader& input_json);

    ~script();

    const std::string& get_next_url() const { return messages.front().url; };
    const std::string& get_next_body() const { return messages.front().body; };
    const std::string& get_next_method() const { return messages.front().method; };
    const std::string& get_next_msg_name() const { return messages.front().id; };
    const msg_headers& get_next_headers() const { return messages.front().headers; };

    const range_type& get_ranges() const { return ranges; };
    const std::string& get_server_dns() const { return server.dns; };
    const std::string& get_server_port() const { return server.port; };
    bool is_server_secure() const { return server.secure; };
    int get_timeout_ms() const { return timeout_ms; };

    bool post_process(const answer_type& last_answer);
    bool validate_answer(const answer_type& last_answer) const;

    void parse_ranges(const std::map<std::string, int64_t, std::less<>>& current);
    void parse_variables();

    std::vector<std::string> get_message_names() const;

    void start_span();
    void start_sleep_span();
    void stop_sleep_span();

    const otel_std::shared_ptr<otel_trace::Span>& get_span() const { return span; };

private:
    void validate_members() const;
    void build(const std::string& input_json);

    bool process_next(const answer_type& last_answer);
    bool save_from_answer(const answer_type& answer, const msg_modifier& sfa);
    bool add_to_request(const std::map<std::string, body_modifier, std::less<>>& atb, message& m);

    bool is_last() const { return messages.size() == 1; };
    void replace_in_messages(const std::string& old_str, const std::string& new_str);

    std::deque<message> messages;
    range_type ranges;
    server_info server;
    int timeout_ms;

    std::map<std::string, std::string, std::less<>> vars;
    std::map<std::string, std::string, std::less<>> saved_strs;
    std::map<std::string, int, std::less<>> saved_ints;
    std::map<std::string, json_reader, std::less<>> saved_jsons;

    otel_std::shared_ptr<otel_trace::Span> span;
    otel_std::shared_ptr<otel_trace::Span> sleep_span;

};
}  // namespace traffic
