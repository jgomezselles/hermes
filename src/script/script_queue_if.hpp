#include <optional>

#include "script_structs.hpp"

#pragma once

namespace traffic
{
class script;

class script_queue_if
{
public:
    virtual ~script_queue_if() = default;
    virtual std::optional<script> get_next_script() = 0;
    virtual void enqueue_script(script s, const answer_type& last_answer) = 0;
    virtual void cancel_script() = 0;
    virtual bool has_pending_scripts() const = 0;
    virtual void close_window() = 0;
    virtual bool is_window_closed() = 0;
};
}  // namespace traffic
