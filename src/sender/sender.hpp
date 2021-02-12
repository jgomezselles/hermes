#include <atomic>
#include <boost/bind.hpp>
#include <future>

#pragma once

namespace config
{
class params;
}
namespace http2_client
{
class client;
}

namespace engine
{
class timer;

class sender
{
public:
    sender() = delete;
    sender(std::unique_ptr<engine::timer> &&t, std::unique_ptr<http2_client::client> &&c,
           std::shared_ptr<config::params> params, std::promise<void> &&p);

    ~sender() = default;

    void send();

private:
    bool still_in_window();
    bool continue_sending();
    std::unique_ptr<engine::timer> timer;
    std::atomic<int64_t> counter;

    std::unique_ptr<http2_client::client> client;
    std::shared_ptr<config::params> params;
    std::promise<void> prom;
};
}  // namespace engine
