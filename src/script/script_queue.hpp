#include <atomic>
#include <deque>
#include <mutex>
#include <shared_mutex>
#include <utility>

#include "script_queue_if.hpp"
#include "script.hpp"

#pragma once

namespace traffic
{
class script_queue : public script_queue_if
{
    using mutex_type = std::shared_timed_mutex;
    using read_lock = std::shared_lock<mutex_type>;
    using write_lock = std::unique_lock<mutex_type>;

public:
    script_queue() = delete;
    script_queue(const script& s)
        : new_script(std::make_unique<script>(s)), scripts(), in_flight(0), window_closed(false)
    {
    }

    ~script_queue() = default;

    boost::optional<script> get_next_script() override;
    void enqueue_script(script s, const answer_type& last_answer) override;
    void cancel_script() override { --in_flight; };
    bool has_pending_scripts() const override { return in_flight != 0; };
    void close_window() override { window_closed.store(true); };
    bool is_window_closed() override { return window_closed.load(); }

private:
    void update_currents_in_range(const range_type& ranges);

    std::unique_ptr<script> new_script;
    std::deque<script> scripts;
    std::atomic<int64_t> in_flight;
    std::atomic<bool> window_closed;
    mutable mutex_type rw_mutex;

protected:
    std::map<std::string, int64_t> current_in_range;
};
}  // namespace traffic
