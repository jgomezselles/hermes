#include "script_reader.hpp"

#include "script_schema.hpp"

namespace traffic
{
script_reader::script_reader(std::istream &file) : json_reader(file, ::script::schema) {}

script_reader::script_reader(const std::string &json) : json_reader(json, ::script::schema) {}

script_reader::script_reader(json_reader &&other) : json_reader(std::move(other)) {}

range_type script_reader::build_ranges()
{
    range_type ranges_to_build;

    if (is_present("/ranges"))
    {
        json_reader jr_ranges{get_value<json_reader>("/ranges")};
        const auto read_ranges = jr_ranges.get_attributes();
        for (const auto &r_name : read_ranges)
        {
            std::pair<int, int> range_pair = {get_value<int>("/ranges/" + r_name + "/min"),
                                              get_value<int>("/ranges/" + r_name + "/max")};

            if (range_pair.first > range_pair.second)
            {
                throw std::logic_error(
                    "Script: This is not gonna work!"
                    "min cannot be greater than max!");
            }

            ranges_to_build.emplace(r_name, range_pair);
        }
    }

    return ranges_to_build;
}

std::deque<message> script_reader::build_messages()
{
    std::deque<message> messages_to_build;

    const auto read_messages = get_value<std::vector<std::string>>("/flow");

    for (const auto &message : read_messages)
    {
        if (message == "Total")
        {
            throw std::logic_error(
                "Error in script!"
                "Message name 'Total' is reserved. "
                "Please, choose another name for your message.");
        }
        script_reader jr_msg{get_value<json_reader>("/messages/" + message)};
        messages_to_build.push_back(jr_msg.build_message(message));
    }
    return messages_to_build;
}

message script_reader::build_message(const std::string &m)
{
    message parsed_message;
    parsed_message.id = m;

    parsed_message.url = get_value<std::string>("/url");
    parsed_message.body = get_json_as_string("/body");
    parsed_message.method = get_value<std::string>("/method");
    parsed_message.pass_code = get_value<int>("/response/code");

    if (is_present("/save_from_answer"))
    {
        script_reader jr_sfa{get_value<json_reader>("/save_from_answer")};
        parsed_message.sfa = jr_sfa.build_message_modifier();
    }

    if (is_present("/add_from_saved_to_body"))
    {
        script_reader jr_atb{get_value<json_reader>("/add_from_saved_to_body")};
        parsed_message.atb = jr_atb.build_message_modifier();
    }

    return parsed_message;
}

msg_modifier script_reader::build_message_modifier()
{
    msg_modifier mm;
    mm.name = get_value<std::string>("/name");
    mm.value_type = get_value<std::string>("/value_type");
    mm.path = get_value<std::string>("/path");
    return mm;
}

server_info script_reader::build_server_info()
{
    server_info server;
    server.dns = get_value<std::string>("/dns");
    server.port = get_value<std::string>("/port");

    if (is_present("/secure"))
    {
        server.secure = get_value<bool>("/secure");
    } else
    {
        server.secure = false;
    }

    return server;
}

int script_reader::build_timeout()
{
    return get_value<int>("/timeout");
}

}  // namespace traffic
