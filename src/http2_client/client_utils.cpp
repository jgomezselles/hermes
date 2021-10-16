#include "client_utils.hpp"

#include "script.hpp"

namespace http2_client
{
std::string build_uri(const std::string& host, const std::string& port, const std::string& uri_path)
{
    return "http://" + host + ":" + port + "/" + uri_path;
}

header_map build_headers(const std::size_t s, const traffic::msg_headers& h)
{
    header_map map = {{CONTENT_TYPE, {APP_JSON, false}},
                      {CONTENT_LENGTH, {std::to_string(s), false}}};

    for (const auto& [k, v] : h)
    {
        map.emplace(k, header_value{v, false});
    }

    return map;
}

request get_next_request(const std::string& host, const std::string& port, const traffic::script& s)
{
    const std::string body = s.get_next_body();
    const auto map = build_headers(body.size(), s.get_next_headers());

    return request{build_uri(host, port, s.get_next_url()), s.get_next_method(), body, map,
                   s.get_next_msg_name()};
}

}  // namespace http2_client