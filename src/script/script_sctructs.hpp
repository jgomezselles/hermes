#pragma once

#include <boost/optional.hpp>
#include <deque>
#include <map>

namespace traffic
{
// name_to_overwrite(min, max)
using range_type = std::map<std::string, std::pair<int, int>>;

using answer_type = std::pair<int, std::string>;

struct msg_modifier
{
    std::string name;
    std::string path;
    std::string value_type;
};

struct message
{
    std::string id;
    std::string url;
    std::string body;
    std::string method;
    int pass_code;
    boost::optional<msg_modifier> sfa;
    boost::optional<msg_modifier> atb;
};

struct server_info
{
    std::string dns;
    std::string port;
    bool secure;
};
}  // namespace traffic
