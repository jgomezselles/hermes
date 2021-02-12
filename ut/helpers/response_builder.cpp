#include "response_builder.h"

#include <functional>
#include <map>
#include <sstream>
#include <string>

namespace ut_helpers
{
response_builder::response_builder(const std::optional<int> &code)
{
    manipulate_int("code", code);
}

}  // namespace ut_helpers