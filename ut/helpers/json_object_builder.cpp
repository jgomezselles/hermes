#include "json_object_builder.hpp"

#include <sstream>

namespace ut_helpers
{
std::string json_object_builder::build() const
{
    std::stringstream building_stream;
    building_stream << "{";
    for (const auto &[key, value] : values)
    {
        building_stream << "\"" << key << "\":" << value << ",";
    }
    building_stream.seekp(-1, building_stream.cur);
    building_stream << "}";

    return building_stream.str();
}

void json_object_builder::manipulate_element(const std::string &key, bool ready,
                                             std::function<std::string()> retriever)
{
    if (ready)
    {
        add_element(key, retriever());
    }
    else
    {
        rem_element(key);
    }
}

json_object_builder &json_object_builder::add_element(const std::string &key,
                                                      const std::string &value)
{
    values[key] = value;
    return *this;
}

json_object_builder &json_object_builder::rem_element(const std::string &key)
{
    values.erase(key);
    return *this;
}

json_object_builder &json_object_builder::manipulate_string(
    const std::string &key, const std::optional<std::string> &element)
{
    manipulate_element(key, element.has_value(),
                       [element]() { return '"' + element.value() + '"'; });
    return *this;
}

json_object_builder &json_object_builder::manipulate_int(const std::string &key,
                                                         const std::optional<int> &element)
{
    manipulate_element(key, element.has_value(),
                       [element]() { return std::to_string(element.value()); });
    return *this;
}

json_object_builder &json_object_builder::manipulate_object(
    const std::string &key, const std::optional<std::string> &element)
{
    manipulate_element(key, element.has_value(), [element]() { return element.value(); });
    return *this;
}

}  // namespace ut_helpers