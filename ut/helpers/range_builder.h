#ifndef HERMES_RANGE_BUILDER_H
#define HERMES_RANGE_BUILDER_H

#include <optional>

#include "json_element_builder.h"

namespace ut_helpers
{
class range_builder : public json_element_builder
{
public:
    range_builder(const std::string &name = "range1");

    range_builder &min(const std::optional<int> &min);

    range_builder &max(const std::optional<int> &max);
};
}  // namespace ut_helpers

#endif
