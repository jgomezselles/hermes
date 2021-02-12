#include "message_builder.h"

#include <functional>
#include <map>
#include <sstream>
#include <string>

#include "json_element_builder.h"
#include "json_object_builder.h"
#include "response_builder.h"

namespace ut_helpers
{
message_builder::message_builder(const std::string &name) : json_element_builder(name)
{
    default_values();
}

message_builder::message_builder(int sequence_number)
    : json_element_builder(std::string("test") + std::to_string(sequence_number))
{
    default_values();
}

message_builder &
message_builder::method(const std::optional<std::string> &method)
{
    manipulate_string("method", method);
    return *this;
}

message_builder &
message_builder::response(const std::optional<std::string> &response)
{
    manipulate_object("response", response);
    return *this;
}

message_builder &
message_builder::body(const std::optional<std::string> &body)
{
    manipulate_object("body", body);
    return *this;
}

void
message_builder::default_values()
{
    method("GET");
    response(response_builder(200).build());
    body("{}");
    url("/v1/test");
}

message_builder &
message_builder::url(const std::optional<std::string> &url)
{
    manipulate_string("url", url);
    return *this;
}

message_builder &
message_builder::sfa(const std::optional<std::string> &sfa)
{
    manipulate_object("save_from_answer", sfa);
    return *this;
}

message_builder &
message_builder::atb(const std::optional<std::string> &atb)
{
    manipulate_object("add_from_saved_to_body", atb);
    return *this;
}
}  // namespace ut_helpers