#pragma once

#include "json_reader.hpp"

namespace traffic
{
class script_reader : public json_reader
{
public:
    script_reader(std::istream &file);

    script_reader(const std::string &json);

    server_info build_server_info();

    int build_timeout();

    range_type build_ranges();

    std::deque<message> build_messages();

    message build_message(const std::string &m);

    msg_modifier build_message_modifier();

private:
    script_reader(json_reader &&mgr);
};

}  // namespace traffic
