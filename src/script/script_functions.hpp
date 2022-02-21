#pragma once

#include <nghttp2/asio_http2.h>

#include <map>
#include <set>
#include <string>

namespace traffic
{
template <typename T>
void check_repeated(std::set<std::string, std::less<>>& unique_ids,
                    const std::map<std::string, T, std::less<>>& map)
{
    for (const auto& [k, _] : map)
    {
        if (!(unique_ids.insert(k)).second)
        {
            throw std::invalid_argument(
                k + " found as repeated variable. Please, choose a different name.");
        }
    }
}

void save_headers(const std::map<std::string, std::string, std::less<>>& headers2save,
                  const nghttp2::asio_http2::header_map& answer_headers,
                  std::map<std::string, std::string, std::less<>>& vars);

void save_body_fields(const std::map<std::string, std::string, std::less<>>& fields2save,
                      const std::string& body_str,
                      std::map<std::string, std::string, std::less<>>& vars);

}  // namespace traffic
