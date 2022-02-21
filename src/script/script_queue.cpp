#include "script_queue.hpp"

#include <optional>

namespace traffic
{
void script_queue::update_currents_in_range(const range_type& ranges)
{
    for (const auto& [k, v] : ranges)
    {
        const auto& current = current_in_range.find(k);
        if (current == current_in_range.end())
        {
            current_in_range.try_emplace(k, v.first);
        }
        else
        {
            current_in_range[k] = current->second + 1 <= v.second ? current->second + 1 : v.first;
        }
    }
}

std::optional<script> script_queue::get_next_script()
{
    write_lock wr_lock(rw_mutex);
    if (!scripts.empty())
    {
        script s = scripts.front();
        scripts.pop_front();
        return s;
    }

    if (!window_closed)
    {
        script script_to_start(*new_script);
        update_currents_in_range(script_to_start.get_ranges());
        script_to_start.parse_ranges(current_in_range);
        script_to_start.parse_variables();
        ++in_flight;
        return script_to_start;
    }

    return std::nullopt;
}

void script_queue::enqueue_script(script s, const answer_type& last_answer)
{
    if (!s.post_process(last_answer))
    {
        --in_flight;
        return;
    }

    write_lock wr_lock(rw_mutex);
    scripts.push_back(s);
}
}  // namespace traffic
