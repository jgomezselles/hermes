#include "timer_impl.hpp"

namespace engine
{
timer_impl::timer_impl(boost::asio::io_context &io_ctx) : timer(io_ctx) {}

void
timer_impl::async_wait(engine::timer::wait_handler &&handler)
{
    this->timer.async_wait(handler);
}

size_t
timer_impl::expires_after(const engine::timer::duration_us expiry_time)
{
    return this->timer.expires_after(expiry_time);
}

}  // namespace engine
