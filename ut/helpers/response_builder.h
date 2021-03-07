#pragma once

#include <optional>

#include "json_object_builder.h"

namespace ut_helpers
{
class response_builder : public json_object_builder
{
public:
    response_builder(const std::optional<int>& code);
};
}  // namespace ut_helpers
