#pragma once

#include <boost/system/error_code.hpp>
#include <chrono>

namespace engine
{
class timer
{
public:
    using wait_handler = std::function<void(boost::system::error_code)>;
    using duration_us = std::chrono::microseconds;

    virtual ~timer() {}

    virtual void async_wait(wait_handler &&handler) = 0;

    virtual std::size_t expires_after(const duration_us expiry_time) = 0;
};
}  // namespace engine
