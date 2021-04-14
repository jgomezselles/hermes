
#include <string>

#pragma once

namespace stats
{
class stats_if
{
public:
    virtual ~stats_if() = default;

    virtual void increase_sent(const std::string& id) = 0;
    virtual void add_measurement(const std::string& id, const int64_t time, const int code) = 0;
    virtual void add_timeout(const std::string& id) = 0;
    virtual void add_error(const std::string& id, const int e) = 0;
    virtual void add_client_error(const std::string& id, const int e) = 0;
};
}  // namespace stats
