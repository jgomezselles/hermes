#pragma once

#include <nghttp2/asio_http2.h>

#include <mutex>
#include <string>

#include "script_structs.hpp"

namespace traffic
{
class script;
}

namespace http2_client
{
using nghttp2::asio_http2::header_map;
using nghttp2::asio_http2::header_value;

inline static const std::string CONTENT_TYPE = "content-type";
inline static const std::string CONTENT_LENGTH = "content-length";
inline static const std::string APP_JSON = "application/json";

struct race_control
{
    race_control() = default;
    bool timed_out = false;
    bool answered = false;
    std::mutex mtx;
};

struct request
{
    std::string url;
    std::string method;
    std::string body;
    nghttp2::asio_http2::header_map headers;
    std::string name;
};

enum class method
{
    GET,
    PUT,
    POST,
    DELETE
};

std::string build_uri(const std::string& host, const std::string& port,
                      const std::string& uri_path);
header_map build_headers(const std::size_t s, const traffic::msg_headers& h);
request get_next_request(const std::string& host, const std::string& port,
                         const traffic::script& s);

}  // namespace http2_client