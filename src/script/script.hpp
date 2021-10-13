#include <iostream>
#include <utility>
#include <vector>

#include "json_reader.hpp"
#include "script_sctructs.hpp"

#pragma once

namespace traffic
{
class script
{
public:
    script() = delete;
    script(const std::string& path);
    script(const json_reader& input_json);

    ~script() = default;

    const std::string& get_next_url() const { return messages.front().url; };
    const std::string& get_next_body() const { return messages.front().body; };
    const std::string& get_next_method() const { return messages.front().method; };
    const std::string& get_next_msg_name() const { return messages.front().id; };

    const range_type& get_ranges() const { return ranges; };
    const std::string& get_server_dns() const { return server.dns; };
    const std::string& get_server_port() const { return server.port; };
    const bool is_server_secure() const { return server.secure; };
    const int get_timeout_ms() const { return timeout_ms; };

    const bool post_process(const answer_type& last_answer);
    bool validate_answer(const answer_type& last_answer) const;

    void parse_ranges(const std::map<std::string, int64_t>& current);
    void parse_variables();

    const std::vector<std::string> get_message_names() const;

private:
    void build(std::istream& file);
    void build(const json_reader& input_json);

    bool process_next(const std::string& last_answer);
    bool save_from_answer(const std::string& answer, const msg_modifier& sfa);
    bool add_to_request(const msg_modifier& atb, message& m);
    const bool is_last() const { return messages.size() == 1; };
    void replace_in_messages(const std::string& old_str, const std::string& new_str);

    std::deque<message> messages;
    range_type ranges;
    server_info server;
    int timeout_ms;

    std::map<std::string, std::string> vars;
    std::map<std::string, std::string> saved_strs;
    std::map<std::string, int> saved_ints;
    std::map<std::string, json_reader> saved_jsons;
};
}  // namespace traffic
