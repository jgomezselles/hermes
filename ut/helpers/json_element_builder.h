#ifndef HERMES_JSON_ELEMENT_BUILDER_H
#define HERMES_JSON_ELEMENT_BUILDER_H

#include <string>

#include "json_object_builder.h"

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

#endif
