#include <chrono>
#pragma once

using std::chrono::steady_clock;
using std::chrono::time_point;
namespace config
{
class params
{
public:
    params() = delete;
    params(const int wait_time, const int duration)
        : wait_time(wait_time), duration(duration), init_time(steady_clock::now())
    {
    }
    params(const params& p) = default;

    ~params() = default;

    int64_t wait_time;
    int64_t duration;
    time_point<steady_clock> init_time;
};
}  // namespace config
