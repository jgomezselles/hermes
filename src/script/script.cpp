#include "script.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <deque>
#include <exception>
#include <fstream>
#include <iostream>
#include <vector>

#include "script_reader.hpp"

namespace traffic
{
script::script(const std::string& path)
{
    std::ifstream json_file(path);
    if (!json_file)
    {
        throw std::logic_error("File " + path +
                               " not found."
                               "Terminating application.");
    }
    this->build(json_file);
}

script::script(std::istream& input)
{
    this->build(input);
}

void script::build(std::istream& file)
{
    script_reader jr{file};
    ranges = jr.build_ranges();
    messages = jr.build_messages();
    server = jr.build_server_info();
    timeout_ms = jr.build_timeout();
}

const std::vector<std::string> script::get_message_names() const
{
    std::vector<std::string> res;
    for (const auto& m : messages)
    {
        res.push_back(m.id);
    }

    return res;
}

bool script::save_from_answer(const std::string& answer, const msg_modifier& sfa)
{
    try
    {
        json_reader ans_json{answer, "{}"};
        if (sfa.value_type == "string")
        {
            saved_strs[sfa.name] = ans_json.get_value<std::string>(sfa.path);
        }
        else if (sfa.value_type == "int")
        {
            saved_ints[sfa.name] = ans_json.get_value<int>(sfa.path);
        }
        else if (sfa.value_type == "object")
        {
            saved_jsons[sfa.name] = ans_json.get_value<json_reader>(sfa.path);
        }
    }
    catch (std::logic_error& le)
    {
        return false;
    }
    return true;
}

bool script::add_to_request(const msg_modifier& atb, message& m)
{
    json_reader modified_body(m.body, "{}");

    try
    {
        if (atb.value_type == "string")
        {
            modified_body.set(atb.path, saved_strs.at(atb.name));
        }
        else if (atb.value_type == "int")
        {
            modified_body.set(atb.path, saved_ints.at(atb.name));
        }
        else if (atb.value_type == "object")
        {
            modified_body.set(atb.path, saved_jsons.at(atb.name));
        }
        m.body = modified_body.as_string();
    }
    catch (std::out_of_range& oor)
    {
        return false;
    }
    return true;
}

bool script::process_next(const std::string& last_answer)
{
    // TODO: if this is an error, validation should fail. Rethink
    const auto& last_msg = messages.front();
    if (last_msg.sfa.is_initialized())
    {
        if (!save_from_answer(last_answer, last_msg.sfa.get()))
        {
            return false;
        }
    }

    messages.pop_front();

    auto& next_msg = messages.front();
    if (next_msg.atb.is_initialized())
    {
        if (!add_to_request(next_msg.atb.get(), next_msg))
        {
            return false;
        }
    }

    return true;
}

bool script::validate_answer(const answer_type& last_answer) const
{
    return last_answer.first == messages.front().pass_code;
}

const bool script::post_process(const answer_type& last_answer)
{
    return !is_last() && process_next(last_answer.second);
}

void script::parse_ranges(const std::map<std::string, int64_t>& current)
{
    for (const auto& c : current)
    {
        for (auto& m : messages)
        {
            std::string str_to_replace = "<" + c.first + ">";
            boost::replace_all(m.body, str_to_replace, std::to_string(c.second));
            boost::replace_all(m.url, str_to_replace, std::to_string(c.second));
        }
    }
}

}  // namespace traffic
