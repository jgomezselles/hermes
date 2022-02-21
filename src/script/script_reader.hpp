#pragma once

#include <map>

#include "json_reader.hpp"

namespace traffic
{
class script_reader
{
public:
    script_reader(const std::string &json);

    server_info build_server_info();
    int build_timeout();
    range_type build_ranges();
    std::deque<message> build_messages();
    message build_message(const std::string &m);
    msg_headers build_message_headers();
    body_modifier build_body_modifier();
    msg_modifier build_sfa();
    std::map<std::string, body_modifier, std::less<>> build_atb();
    std::map<std::string, std::string, std::less<>> build_variables();

private:
    script_reader(json_reader &&mgr);
    json_reader json_rdr;
};

}  // namespace traffic
