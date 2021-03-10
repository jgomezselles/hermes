#include "script_builder.hpp"

#include "json_element_builder.hpp"
#include "message_builder.hpp"
#include "range_builder.hpp"

namespace ut_helpers
{
script_builder::script_builder()
{
    dns("public-dns");
    port("8686");
    timeout(2000);
    messages(std::vector<std::string>{message_builder().build()});
    flow(std::vector<std::string>{"\"test1\""});
    ranges(std::vector<std::string>{range_builder().build()});
}

script_builder &script_builder::dns(const std::optional<std::string> &dns)
{
    manipulate_string("dns", dns);
    return *this;
}

script_builder &script_builder::port(const std::optional<std::string> &port)
{
    manipulate_string("port", port);
    return *this;
}

script_builder &script_builder::timeout(const std::optional<int> &timeout)
{
    manipulate_int("timeout", timeout);
    return *this;
}

script_builder &script_builder::messages(const std::optional<std::vector<std::string>> &messages)
{
    add_composed_objects("messages", messages);
    return *this;
}

script_builder &script_builder::flow(const std::optional<std::vector<std::string>> &flows)
{
    add_composed_objects("flow", flows, "[]");
    return *this;
}

script_builder &script_builder::ranges(const std::optional<std::vector<std::string>> &ranges)
{
    add_composed_objects("ranges", ranges);
    return *this;
}

void script_builder::add_composed_objects(const std::string &key,
                                          const std::optional<std::vector<std::string>> &elements,
                                          const char *delimiters)
{
    manipulate_element(key, elements.has_value(), [&elements, &delimiters]() {
        std::stringstream building;
        building << delimiters[0];
        for (const auto &message : elements.value())
        {
            building << message << ",";
        }
        building.seekp(-1, building.cur);
        building << delimiters[1];
        return building.str();
    });
}

}  // namespace ut_helpers
