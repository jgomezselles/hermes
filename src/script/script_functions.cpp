#include "script_functions.hpp"

#include <nghttp2/asio_http2.h>

#include <map>
#include <set>
#include <string>

#include "json_reader.hpp"

namespace traffic
{
void save_headers(const std::map<std::string, std::string>& headers2save,
                  const nghttp2::asio_http2::header_map& answer_headers,
                  std::map<std::string, std::string>& vars)
{
    for (const auto& [id, header_field] : headers2save)
    {
        const auto& header_to_save = answer_headers.find(header_field);
        if (header_to_save == answer_headers.end())
        {
            throw std::logic_error("Header " + header_field + " not found.");
        }
        vars.insert_or_assign(id, header_to_save->second.value);
    }
}

void save_body_fields(const std::map<std::string, std::string>& fields2save,
                      const std::string& body_str, std::map<std::string, std::string>& vars)
{
    json_reader body_json{body_str, ""};
    for (const auto& [id, path] : fields2save)
    {
        vars.insert_or_assign(id, body_json.get_json_as_string(path));
    }
}

}  // namespace traffic
