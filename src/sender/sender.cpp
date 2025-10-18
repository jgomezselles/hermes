#include "sender.hpp"

#include <boost/bind/bind.hpp>

#include "client_impl.hpp"
#include "params.hpp"
#include "timer.hpp"

using namespace std::chrono;

namespace engine
{
sender::sender(std::unique_ptr<engine::timer>&& t, std::unique_ptr<http2_client::client>&& c,
               std::shared_ptr<config::params> params, std::promise<void>&& p)
    : timer(std::move(t)), counter(0), client(std::move(c)), params(params), prom(std::move(p))
{
    timer->async_wait(boost::bind(&sender::send, this));
}

bool sender::still_in_window()
{
    int seconds_since_start = duration_cast<seconds>(steady_clock::now() - params->init_time)
                                  .count();  // pass this one to microseconds
    bool in_window = seconds_since_start < params->duration;
    if (!in_window)
    {
        client->close_window();
    }

    return in_window;
}

bool sender::continue_sending()
{
    return still_in_window() || !client->has_finished();
}

void sender::send()
{
    if (continue_sending())
    {
        ++counter;
        steady_clock::time_point future_time =
            (params->init_time + std::chrono::microseconds(params->wait_time * counter.load()));
        auto elapsed = duration_cast<microseconds>(future_time - steady_clock::now()).count();
        timer->expires_after(microseconds(elapsed));
        timer->async_wait(boost::bind(&sender::send, this));
    }
    else
    {
        prom.set_value();
        return;
    }

    client->send();
}

}  // namespace engine
