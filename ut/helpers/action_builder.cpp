#include "action_builder.h"

#include <functional>
#include <map>
#include <optional>
#include <sstream>
#include <string>

namespace ut_helpers
{
action_builder &action_builder::name(const std::optional<std::string> &name)
{
    manipulate_string("name", name);
    return *this;
}

action_builder &action_builder::path(const std::optional<std::string> &path)
{
    manipulate_string("path", path);
    return *this;
}

action_builder &action_builder::value_type(const std::optional<std::string> &value_type)
{
    manipulate_string("value_type", value_type);
    return *this;
}
}  // namespace ut_helpers