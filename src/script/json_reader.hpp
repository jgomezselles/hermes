#pragma once

#include <iostream>
#include <vector>

#include "rapidjson/document.h"
#include "script_structs.hpp"

namespace traffic
{
class json_reader
{
public:
    json_reader() = default;
    ~json_reader() = default;
    json_reader(const std::string& json, const std::string& schema_str);
    json_reader(const json_reader& other);
    friend void swap(json_reader& first, json_reader& second) noexcept;
    json_reader& operator=(json_reader other);
    bool operator==(const json_reader& other) const;

    template <typename t>
    t get_value(const std::string& path)
    {
        throw std::invalid_argument("Type not implemented when asked for " + path);
    }

    template <typename t>
    void set(const std::string& path, const t&)
    {
        throw std::invalid_argument("Type not implemented when asked for " + path);
    }

    void erase(const std::string& path);

    bool is_present(const std::string& path);
    std::vector<std::string> get_attributes();
    std::string get_json_as_string(const std::string& path);
    std::string as_string() const;

    bool is_string(const std::string& path);
    bool is_number(const std::string& path);

private:
    explicit json_reader(const rapidjson::Value* value);
    void parse(const std::string& json_str, const std::string& schema_str);

    rapidjson::Document document;
};

template <>
json_reader json_reader::get_value<json_reader>(const std::string& path);

template <>
int json_reader::get_value<int>(const std::string& path);

template <>
bool json_reader::get_value<bool>(const std::string& path);

template <>
std::string json_reader::get_value<std::string>(const std::string& path);

template <>
std::vector<std::string> json_reader::get_value<std::vector<std::string>>(const std::string& path);

template <>
void json_reader::set<int>(const std::string& path, const int& value);

template <>
void json_reader::set<bool>(const std::string& path, const bool& value);

template <>
void json_reader::set<std::string>(const std::string& path, const std::string& value);

template <>
void json_reader::set<json_reader>(const std::string& path, const json_reader& value);

template <>
void json_reader::set<std::vector<std::string>>(const std::string& path,
                                                const std::vector<std::string>& value);

}  // namespace traffic
