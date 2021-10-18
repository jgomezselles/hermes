#pragma once

#include <deque>
#include <map>
#include <optional>

namespace traffic
{
// name_to_overwrite(min, max)
using range_type = std::map<std::string, std::pair<int, int>>;
using answer_type = std::pair<int, std::string>;
using msg_headers = std::map<std::string, std::string>;

struct msg_modifier
{
    std::string name;
    std::string path;
    std::string value_type;
};

struct msg_modifier_v2
{
    // id, header_field
    std::map<std::string, std::string> headers;
    // id, path
    std::map<std::string, std::string> body_fields;
};

struct message
{
    std::string id;
    std::string url;
    std::string body;
    std::string method;
    int pass_code;
    msg_headers headers;

    std::optional<msg_modifier> sfa;
    std::optional<msg_modifier> atb;

    msg_modifier_v2 save;
};

struct server_info
{
    std::string dns;
    std::string port;
    bool secure;
};
}  // namespace traffic
