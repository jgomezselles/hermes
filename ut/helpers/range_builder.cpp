#include "range_builder.h"

#include <functional>
#include <map>
#include <string>

#include "json_element_builder.h"

namespace ut_helpers
{
range_builder::range_builder(const std::string &name) : json_element_builder(name)
{
    min(0);
    max(1);
}

range_builder &
range_builder::min(const std::optional<int> &min)
{
    manipulate_int("min", min);
    return *this;
}

range_builder &
range_builder::max(const std::optional<int> &max)
{
    manipulate_int("max", max);
    return *this;
}
}  // namespace ut_helpers