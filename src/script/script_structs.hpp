#pragma once

#include <nghttp2/asio_http2.h>

#include <deque>
#include <map>
#include <optional>

namespace nghttp2::asio_http2
{
static inline bool operator==(const nghttp2::asio_http2::header_value& lhs,
                              const nghttp2::asio_http2::header_value& rhs)
{
    return lhs.value == rhs.value && lhs.sensitive == rhs.sensitive;
};
}  // namespace nghttp2::asio_http2

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

    bool operator==(const answer_type& other) const
    {
        return result_code == other.result_code && body == other.body && headers != other.headers;
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
