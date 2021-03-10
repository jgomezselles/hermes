#pragma once

#include <string>

#include "json_object_builder.hpp"

namespace ut_helpers
{
class json_element_builder : public json_object_builder
{
public:
    json_element_builder(const std::string& name);
    virtual std::string build() const override;

protected:
    std::string name;
};
}  // namespace ut_helpers
