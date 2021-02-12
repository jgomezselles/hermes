#ifndef HERMES_MESSAGE_BUILDER_H
#define HERMES_MESSAGE_BUILDER_H

#include <optional>

#include "json_element_builder.h"

namespace ut_helpers
{
class message_builder : public json_element_builder
{
public:
    message_builder(const std::string& name = "test1");
    message_builder(int sequence_number);

    message_builder& method(const std::optional<std::string>& method);
    message_builder& response(const std::optional<std::string>& response);
    message_builder& url(const std::optional<std::string>& url);
    message_builder& body(const std::optional<std::string>& body);
    message_builder& sfa(const std::optional<std::string>& sfa);
    message_builder& atb(const std::optional<std::string>& atb);

private:
    void default_values();
};
}  // namespace ut_helpers

#endif