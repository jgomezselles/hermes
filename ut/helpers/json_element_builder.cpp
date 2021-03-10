#include "json_element_builder.hpp"

#include <functional>
#include <map>
#include <sstream>
#include <string>

#include "json_object_builder.hpp"

namespace ut_helpers
{
std::string json_element_builder::build() const
{
    return '"' + name + '"' + ":" + json_object_builder::build();
}

json_element_builder::json_element_builder(const std::string &name) : name(name) {}
}  // namespace ut_helpers