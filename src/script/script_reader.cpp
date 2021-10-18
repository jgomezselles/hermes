#include "script_reader.hpp"

#include "script_schema.hpp"

namespace traffic
{
script_reader::script_reader(const std::string &json) : json_rdr(json, ::script::schema) {}

script_reader::script_reader(json_reader &&other) : json_rdr(std::move(other)) {}

range_type script_reader::build_ranges()
{
    range_type ranges_to_build;

    if (json_rdr.is_present("/ranges"))
    {
        json_reader jr_ranges{json_rdr.get_value<json_reader>("/ranges")};
        const auto read_ranges = jr_ranges.get_attributes();
        for (const auto &r_name : read_ranges)
        {
            std::pair<int, int> range_pair = {
                json_rdr.get_value<int>("/ranges/" + r_name + "/min"),
                json_rdr.get_value<int>("/ranges/" + r_name + "/max")};

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

    const auto read_messages = json_rdr.get_value<std::vector<std::string>>("/flow");

    for (const auto &message : read_messages)
    {
        if (message == "Total")
        {
            throw std::logic_error(
                "Error in script!"
                "Message name 'Total' is reserved. "
                "Please, choose another name for your message.");
        }
        script_reader jr_msg{json_rdr.get_value<json_reader>("/messages/" + message)};
        messages_to_build.push_back(jr_msg.build_message(message));
    }
    return messages_to_build;
}

msg_headers script_reader::build_message_headers()
{
    msg_headers mh;
    for (const auto &attr : json_rdr.get_attributes())
    {
        mh.emplace(attr, json_rdr.get_value<std::string>("/" + attr));
    }
    return mh;
}

msg_modifier_v2 script_reader::build_message_modifier_v2()
{
    msg_modifier_v2 mm;
    if (json_rdr.is_present("/headers"))
    {
        script_reader sr_headers{json_rdr.get_value<json_reader>("/headers")};
        mm.headers = sr_headers.build_message_headers();
    }

    if (json_rdr.is_present("/body"))
    {
        script_reader sr_headers{json_rdr.get_value<json_reader>("/body")};
        mm.body_fields = sr_headers.build_message_headers();
    }
    return mm;
}

message script_reader::build_message(const std::string &m)
{
    message parsed_message;
    parsed_message.id = m;

    parsed_message.url = json_rdr.get_value<std::string>("/url");
    parsed_message.body = json_rdr.get_json_as_string("/body");
    parsed_message.method = json_rdr.get_value<std::string>("/method");
    parsed_message.pass_code = json_rdr.get_value<int>("/response/code");

    if (json_rdr.is_present("/headers"))
    {
        script_reader sr_headers{json_rdr.get_value<json_reader>("/headers")};
        parsed_message.headers = sr_headers.build_message_headers();
    }

    if (json_rdr.is_present("/save"))
    {
        script_reader sr_save{json_rdr.get_value<json_reader>("/save")};
        parsed_message.save = sr_save.build_message_modifier_v2();
    }

    if (json_rdr.is_present("/save_from_answer"))
    {
        script_reader sr_sfa{json_rdr.get_value<json_reader>("/save_from_answer")};
        parsed_message.sfa = sr_sfa.build_message_modifier();
    }

    if (json_rdr.is_present("/add_from_saved_to_body"))
    {
        script_reader sr_atb{json_rdr.get_value<json_reader>("/add_from_saved_to_body")};
        parsed_message.atb = sr_atb.build_message_modifier();
    }

    return parsed_message;
}

msg_modifier script_reader::build_message_modifier()
{
    msg_modifier mm;
    mm.name = json_rdr.get_value<std::string>("/name");
    mm.value_type = json_rdr.get_value<std::string>("/value_type");
    mm.path = json_rdr.get_value<std::string>("/path");
    return mm;
}

server_info script_reader::build_server_info()
{
    server_info server;
    server.dns = json_rdr.get_value<std::string>("/dns");
    server.port = json_rdr.get_value<std::string>("/port");
    server.secure = json_rdr.is_present("/secure") ? json_rdr.get_value<bool>("/secure") : false;

    return server;
}

int script_reader::build_timeout()
{
    return json_rdr.get_value<int>("/timeout");
}

std::map<std::string, std::string> script_reader::build_variables()
{
    std::map<std::string, std::string> vars;
    if (json_rdr.is_present("/variables"))
    {
        json_reader jr_ranges{json_rdr.get_value<json_reader>("/variables")};
        const auto var_names = jr_ranges.get_attributes();
        for (const auto &key : var_names)
        {
            const std::string full_key{"/variables/" + key};
            if (json_rdr.is_string(full_key))
            {
                vars.emplace(key, json_rdr.get_value<std::string>(full_key));
            }
            else if (json_rdr.is_number(full_key))
            {
                vars.emplace(key, std::to_string(json_rdr.get_value<int>(full_key)));
            }
            else
            {
                throw std::logic_error(
                    "Error in script. Variables can only be strings or integers.");
            }
        }
    }
    return vars;
}

}  // namespace traffic
