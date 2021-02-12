#ifndef HERMES_TIMER_IMPL_H
#define HERMES_TIMER_IMPL_H

#include <boost/asio.hpp>

#include "timer.hpp"

namespace engine
{
class timer_impl : public engine::timer
{
public:
    timer_impl(boost::asio::io_context& io_ctx);

    void async_wait(wait_handler&& handler) override;

    size_t expires_after(duration_us expiry_time) override;

private:
    boost::asio::steady_timer timer;
};

}  // namespace engine

#endif