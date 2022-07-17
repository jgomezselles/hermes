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
                throw std::invalid_argument(
                    "Script: This is not gonna work!"
                    "min cannot be greater than max!");
            }

            ranges_to_build.try_emplace(r_name, range_pair);
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
            throw std::invalid_argument(
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
        mh.try_emplace(attr, json_rdr.get_value<std::string>("/" + attr));
    }
    return mh;
}

std::map<std::string, body_modifier, std::less<>> script_reader::build_atb()
{
    std::map<std::string, body_modifier, std::less<>> bms;
    for (const auto &attr : json_rdr.get_attributes())
    {
        script_reader sr_body_fields{json_rdr.get_value<json_reader>("/" + attr)};
        bms.try_emplace(attr, sr_body_fields.build_body_modifier());
    }
    return bms;
}

msg_modifier script_reader::build_sfa()
{
    msg_modifier mms;
    for (const auto &attr : json_rdr.get_attributes())
    {
        if (attr == "headers")
        {
            script_reader sr_headers{json_rdr.get_value<json_reader>("/headers")};
            mms.headers = sr_headers.build_message_headers();
        }
        else
        {
            script_reader sr_body_fields{json_rdr.get_value<json_reader>("/" + attr)};
            mms.body_fields.try_emplace(attr, sr_body_fields.build_body_modifier());
        }
    }
    return mms;
}

message script_reader::build_message(std::string_view m)
{
    message parsed_message;
    parsed_message.id = m;
    parsed_message.url = json_rdr.get_value<std::string>("/url");
    parsed_message.method = json_rdr.get_value<std::string>("/method");
    parsed_message.pass_code = json_rdr.get_value<int>("/response/code");

    if (json_rdr.is_present("/body"))
    {
        parsed_message.body = json_rdr.get_json_as_string("/body");
    }

    if (json_rdr.is_present("/headers"))
    {
        script_reader sr_headers{json_rdr.get_value<json_reader>("/headers")};
        parsed_message.headers = sr_headers.build_message_headers();
    }

    if (json_rdr.is_present("/save_from_answer"))
    {
        script_reader sr_sfa{json_rdr.get_value<json_reader>("/save_from_answer")};
        parsed_message.sfa = sr_sfa.build_sfa();
    }

    if (json_rdr.is_present("/add_from_saved_to_body"))
    {
        script_reader sr_atb{json_rdr.get_value<json_reader>("/add_from_saved_to_body")};
        parsed_message.atb = sr_atb.build_atb();
    }

    return parsed_message;
}

body_modifier script_reader::build_body_modifier()
{
    body_modifier mm;
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

std::map<std::string, std::string, std::less<>> script_reader::build_variables()
{
    std::map<std::string, std::string, std::less<>> vars;
    if (json_rdr.is_present("/variables"))
    {
        json_reader jr_ranges{json_rdr.get_value<json_reader>("/variables")};
        const auto var_names = jr_ranges.get_attributes();
        for (const auto &key : var_names)
        {
            const std::string full_key{"/variables/" + key};
            if (json_rdr.is_string(full_key))
            {
                vars.try_emplace(key, json_rdr.get_value<std::string>(full_key));
            }
            else if (json_rdr.is_number(full_key))
            {
                vars.try_emplace(key, std::to_string(json_rdr.get_value<int>(full_key)));
            }
        }
    }
    return vars;
}

}  // namespace traffic
