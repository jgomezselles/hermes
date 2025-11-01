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

std::shared_ptr<script> script_queue::get_next_script()
{
    write_lock wr_lock(rw_mutex);
    if (!scripts.empty())
    {
        std::shared_ptr<script> s = std::move(scripts.front());
        s->stop_sleep_span();
        scripts.pop_front();
        return s;
    }

    if (!window_closed)
    {
        auto script_to_start = std::make_shared<script>(*new_script);
        update_currents_in_range(script_to_start->get_ranges());
        script_to_start->parse_ranges(current_in_range);
        script_to_start->parse_variables();
        ++in_flight;
        script_to_start->start_span();
        return script_to_start;
    }

    return nullptr;
}

void script_queue::enqueue_script(std::shared_ptr<script>&& s, const answer_type& last_answer)
{
    if (!s->post_process(last_answer))
    {
        --in_flight;
        return;
    }

    write_lock wr_lock(rw_mutex);
    s->start_sleep_span();
    scripts.push_back(std::move(s));
}
}  // namespace traffic
