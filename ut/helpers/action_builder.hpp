#pragma once

#include "json_object_builder.hpp"

namespace ut_helpers
{
class action_builder : public json_object_builder
{
public:
    action_builder &name(const std::optional<std::string> &name);

    action_builder &path(const std::optional<std::string> &path);

    action_builder &value_type(const std::optional<std::string> &value_type);
};
}  // namespace ut_helpers
