#ifndef HERMES_JSON_READER_H
#define HERMES_JSON_READER_H

#include <iostream>

#include "script_sctructs.hpp"
#include "rapidjson/document.h"

namespace traffic
{
class json_reader
{
public:
    json_reader();
    json_reader(std::istream& file, const std::string& schema);
    json_reader(const std::string& json, const std::string& schema_str);
    json_reader(const json_reader& other);
    friend void swap(json_reader& first, json_reader& second);
    json_reader& operator=(json_reader other);

    template <typename t>
    t get_value(const std::string& path)
    {
        throw std::logic_error("Type not implemented when asked for " + path);
    }

    template <typename t>
    void set(const std::string& path, const t& value)
    {
        throw std::logic_error("Type not implemented when asked for " + path);
    }

    bool is_present(const std::string& path);
    std::vector<std::string> get_attributes();
    std::string get_json_as_string(const std::string& path);
    std::string as_string();

protected:
    rapidjson::Document document;

    json_reader(const rapidjson::Value* value);

private:
    void parse(const std::string& json_str, const std::string& schema_str);
};

template <>
json_reader
json_reader::get_value<json_reader>(const std::string& path);

template <>
int
json_reader::get_value<int>(const std::string& path);

template <>
std::string
json_reader::get_value<std::string>(const std::string& path);

template <>
std::vector<std::string>
json_reader::get_value<std::vector<std::string>>(const std::string& path);

template <>
void
json_reader::set<int>(const std::string& path, const int& value);

template <>
void
json_reader::set<std::string>(const std::string& path, const std::string& value);

template <>
void
json_reader::set<json_reader>(const std::string& path, const json_reader& value);

}  // namespace traffic

#endif
