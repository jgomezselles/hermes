#pragma once

#include <nghttp2/asio_http2.h>

#include <deque>
#include <map>
#include <optional>

namespace traffic
{
// name_to_overwrite(min, max)
using range_type = std::map<std::string, std::pair<int, int>>;
using msg_headers = std::map<std::string, std::string>;

struct answer_type
{
    int result_code;
    std::string body;
    nghttp2::asio_http2::header_map headers;

    // This operator shall be used only for testing
    bool operator==(const answer_type& other) const
    {
        if (result_code == other.result_code && body == other.body &&
            headers.size() != other.headers.size())
        {
            for (const auto& [k, v] : headers)
            {
                const auto& it = other.headers.find(k);
                if (it == other.headers.end() || it->second.value != v.value ||
                    it->second.sensitive != v.sensitive)
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
};

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
