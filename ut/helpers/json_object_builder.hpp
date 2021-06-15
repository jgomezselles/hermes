#pragma once

#include <functional>
#include <map>
#include <optional>
#include <string>

namespace ut_helpers
{
class json_object_builder
{
public:
    json_object_builder& manipulate_string(const std::string& key,
                                           const std::optional<std::string>& element);
    json_object_builder& manipulate_int(const std::string& key, const std::optional<int>& element);
    json_object_builder& manipulate_bool(const std::string& key, const std::optional<bool>& element);
    json_object_builder& manipulate_object(const std::string& key,
                                           const std::optional<std::string>& element);
    json_object_builder& add_element(const std::string& key, const std::string& value);
    json_object_builder& rem_element(const std::string& key);
    virtual std::string build() const;

protected:
    std::map<std::string, std::string> values;

    void manipulate_element(const std::string& key, bool ready,
                            std::function<std::string()> retriever);
};
}  // namespace ut_helpers
