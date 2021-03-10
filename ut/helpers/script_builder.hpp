#pragma once

#include <functional>
#include <map>
#include <sstream>
#include <vector>

#include "json_object_builder.hpp"

namespace ut_helpers
{
class script_builder : public json_object_builder
{
public:
    script_builder();

    script_builder& dns(const std::optional<std::string>& dns);
    script_builder& port(const std::optional<std::string>& port);
    script_builder& timeout(const std::optional<int>& timeout);
    script_builder& messages(const std::optional<std::vector<std::string>>& messages);
    script_builder& flow(const std::optional<std::vector<std::string>>& flow);
    script_builder& ranges(const std::optional<std::vector<std::string>>& ranges);

private:
    void add_composed_objects(const std::string& key,
                              const std::optional<std::vector<std::string>>& elements,
                              const char delimiters[] = "{}");
};

}  // namespace ut_helpers
