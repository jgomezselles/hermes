#include "json_reader.hpp"

#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace traffic
{
void json_reader::parse(const std::string& json_str, const std::string& schema_str)
{
    if (document.Parse(json_str.c_str()).HasParseError())
    {
        throw std::logic_error("Error parsing input! Wrong json.");
    }

    if (schema_str.size())
    {
        rapidjson::Document sd;

        if (sd.Parse(schema_str.c_str()).HasParseError())
        {
            throw std::logic_error("Not valid schema.");
        }
        rapidjson::SchemaDocument schema(sd);
        rapidjson::SchemaValidator validator(schema);

        if (!document.Accept(validator))
        {
            throw std::logic_error("Input does not validate against the schema! ");
        }
    }
}

json_reader::json_reader() {}

json_reader::json_reader(const std::string& json, const std::string& schema)
{
    parse(json, schema);
}

json_reader::json_reader(const rapidjson::Value* value)
{
    document.CopyFrom(*value, document.GetAllocator());
}

void swap(json_reader& first, json_reader& second)
{
    first.document.Swap(second.document);
}

json_reader::json_reader(const json_reader& other)
{
    document.CopyFrom(other.document, document.GetAllocator());
}

json_reader& json_reader::operator=(json_reader other)
{
    swap(*this, other);
    return *this;
}

template <>
json_reader json_reader::get_value<json_reader>(const std::string& path)
{
    const auto* value = rapidjson::Pointer(path.c_str()).Get(document);
    if (value && value->GetType() == rapidjson::kObjectType)
    {
        return json_reader(value);
    }

    throw std::logic_error("Object not found in " + path);
}

template <>
int json_reader::get_value<int>(const std::string& path)
{
    const auto* value = rapidjson::Pointer(path.c_str()).Get(document);
    if (value && value->GetType() == rapidjson::kNumberType)
    {
        return value->GetInt();
    }

    throw std::logic_error("Integer not found in " + path);
}

template <>
bool json_reader::get_value<bool>(const std::string& path)
{
    const auto* value = rapidjson::Pointer(path.c_str()).Get(document);
    if (value &&
        (value->GetType() == rapidjson::kFalseType || value->GetType() == rapidjson::kTrueType))
    {
        return value->GetBool();
    }

    throw std::logic_error("Bool not found in " + path);
}

template <>
std::string json_reader::get_value<std::string>(const std::string& path)
{
    const auto* value = rapidjson::Pointer(path.c_str()).Get(document);
    if (value && value->GetType() == rapidjson::kStringType)
    {
        return value->GetString();
    }

    throw std::logic_error("String not found in " + path);
}

template <>
std::vector<std::string> json_reader::get_value<std::vector<std::string>>(const std::string& path)
{
    const auto value = rapidjson::Pointer(path.c_str()).Get(document);
    if (value && value->GetType() == rapidjson::kArrayType)
    {
        std::vector<std::string> result;
        for (const auto& element : value->GetArray())
        {
            if (!element.IsString())
            {
                throw std::logic_error("String not expected in array under" + path);
            }
            result.push_back(element.GetString());
        }

        return result;
    }

    throw std::logic_error("No value set in " + path);
}

std::vector<std::string> json_reader::get_attributes()
{
    if (!document.IsObject())
    {
        return {};
    }

    std::vector<std::string> attrs;
    for (const auto& attr : document.GetObject())
    {
        attrs.emplace_back(attr.name.GetString());
    }

    return attrs;
}

std::string json_reader::get_json_as_string(const std::string& path)
{
    const auto* value = rapidjson::Pointer(path.c_str()).Get(document);
    if (value)
    {
        rapidjson::StringBuffer buffer;
        buffer.Clear();
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        if (value->Accept(writer))
        {
            return std::string(buffer.GetString());
        }
    }

    throw std::logic_error("No value set in " + path);
}

std::string json_reader::as_string() const
{
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    return std::string(buffer.GetString());
}

template <>
void json_reader::set<int>(const std::string& path, const int& value)
{
    rapidjson::Pointer(path.c_str()).Create(document);
    auto* val = rapidjson::Pointer(path.c_str()).Get(document);
    if (val)
    {
        val->SetInt(value);
        return;
    }

    throw std::logic_error("Error setting integer under " + path);
}

template <>
void json_reader::set<bool>(const std::string& path, const bool& value)
{
    rapidjson::Pointer(path.c_str()).Create(document);
    auto* val = rapidjson::Pointer(path.c_str()).Get(document);
    if (val)
    {
        val->SetBool(value);
        return;
    }

    throw std::logic_error("Error setting integer under " + path);
}

template <>
void json_reader::set<std::string>(const std::string& path, const std::string& value)
{
    rapidjson::Pointer(path.c_str()).Create(document);
    auto* val = rapidjson::Pointer(path.c_str()).Get(document);
    if (val)
    {
        val->SetString(value.c_str(), value.size(), document.GetAllocator());
        return;
    }

    throw std::logic_error("Error setting string under " + path);
}

template <>
void json_reader::set<json_reader>(const std::string& path, const json_reader& value)
{
    rapidjson::Pointer(path.c_str()).Create(document);
    auto* val = rapidjson::Pointer(path.c_str()).Get(document);
    if (val)
    {
        val->CopyFrom(value.document, document.GetAllocator());
        return;
    }

    throw std::logic_error("Error setting object under " + path);
}

template <>
void json_reader::set<std::vector<std::string>>(const std::string& path,
                                                const std::vector<std::string>& values)
{
    rapidjson::Pointer(path.c_str()).Create(document);
    auto* val = rapidjson::Pointer(path.c_str()).Get(document);
    if (val)
    {
        val->SetArray();
        for (const auto& value : values)
        {
            rapidjson::Value v;
            v.SetString(value.c_str(), value.size(), document.GetAllocator());
            val->PushBack(v, document.GetAllocator());
        }
        return;
    }

    throw std::logic_error("Error setting object under " + path);
}

bool json_reader::is_present(const std::string& path)
{
    return rapidjson::Pointer(path.c_str()).Get(document) != nullptr;
}

bool json_reader::is_string(const std::string& path)
{
    const auto* val = rapidjson::Pointer(path.c_str()).Get(document);
    return val->GetType() == rapidjson::kStringType;
}

bool json_reader::is_number(const std::string& path)
{
    const auto* val = rapidjson::Pointer(path.c_str()).Get(document);
    return val->GetType() == rapidjson::kNumberType;
}

void json_reader::erase(const std::string& path)
{
    rapidjson::Pointer(path.c_str()).Erase(document);
}

}  // namespace traffic
