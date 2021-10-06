#include "script_queue.hpp"

#include <optional>

namespace traffic
{
void script_queue::update_currents_in_range(const range_type& ranges)
{
    for (const auto& range : ranges)
    {
        const auto& current = current_in_range.find(range.first);
        if (current == current_in_range.end())
        {
            current_in_range.emplace(range.first, range.second.first);
        }
        else
        {
            current_in_range[range.first] = current->second + 1 <= range.second.second
                                                ? current->second + 1
                                                : range.second.first;
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
