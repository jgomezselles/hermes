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
        throw std::invalid_argument("Error parsing input! Wrong json.");
    }

    if (schema_str.size())
    {
        rapidjson::Document sd;

        if (sd.Parse(schema_str.c_str()).HasParseError())
        {
            throw std::invalid_argument("Not valid schema.");
        }
        rapidjson::SchemaDocument schema(sd);
        rapidjson::SchemaValidator validator(schema);

        if (!document.Accept(validator))
        {
            throw std::invalid_argument("Input does not validate against the schema! ");
        }
    }
}

json_reader::json_reader(const std::string& json, const std::string& schema)
{
    parse(json, schema);
}

json_reader::json_reader(const rapidjson::Value* value)
{
    document.CopyFrom(*value, document.GetAllocator());
}

void swap(json_reader& first, json_reader& second) noexcept
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

bool json_reader::operator==(const json_reader& other) const
{
    return document == other.document;
}

template <>
json_reader json_reader::get_value<json_reader>(const std::string& path)
{
    if (const auto* value = rapidjson::Pointer(path.c_str()).Get(document);
        value && value->GetType() == rapidjson::kObjectType)
    {
        return json_reader(value);
    }

    throw std::out_of_range("Object not found in " + path);
}

template <>
int json_reader::get_value<int>(const std::string& path)
{
    if (const auto* value = rapidjson::Pointer(path.c_str()).Get(document);
        value && value->GetType() == rapidjson::kNumberType)
    {
        return value->GetInt();
    }

    throw std::out_of_range("Integer not found in " + path);
}

template <>
bool json_reader::get_value<bool>(const std::string& path)
{
    if (const auto* value = rapidjson::Pointer(path.c_str()).Get(document);
        value &&
        (value->GetType() == rapidjson::kFalseType || value->GetType() == rapidjson::kTrueType))
    {
        return value->GetBool();
    }

    throw std::out_of_range("Bool not found in " + path);
}

template <>
std::string json_reader::get_value<std::string>(const std::string& path)
{
    if (const auto* value = rapidjson::Pointer(path.c_str()).Get(document);
        value && value->GetType() == rapidjson::kStringType)
    {
        return value->GetString();
    }

    throw std::out_of_range("String not found in " + path);
}

template <>
std::vector<std::string> json_reader::get_value<std::vector<std::string>>(const std::string& path)
{
    if (const auto value = rapidjson::Pointer(path.c_str()).Get(document);
        value && value->GetType() == rapidjson::kArrayType)
    {
        std::vector<std::string> result;
        for (const auto& element : value->GetArray())
        {
            if (!element.IsString())
            {
                throw std::invalid_argument("Expected string but found other in array under " +
                                            path);
            }
            result.emplace_back(element.GetString());
        }

        return result;
    }

    throw std::out_of_range("Array not found in " + path);
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
    if (const auto* value = rapidjson::Pointer(path.c_str()).Get(document); value)
    {
        rapidjson::StringBuffer buffer;
        buffer.Clear();
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

        if (value->Accept(writer))
        {
            return std::string(buffer.GetString());
        }
    }

    throw std::out_of_range("No value set in " + path);
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
    if (auto* val = rapidjson::Pointer(path.c_str()).Get(document); val)
    {
        val->SetInt(value);
        return;
    }

    throw std::out_of_range("Error setting integer under " + path);
}

template <>
void json_reader::set<bool>(const std::string& path, const bool& value)
{
    rapidjson::Pointer(path.c_str()).Create(document);
    if (auto* val = rapidjson::Pointer(path.c_str()).Get(document); val)
    {
        val->SetBool(value);
        return;
    }

    throw std::out_of_range("Error setting integer under " + path);
}

template <>
void json_reader::set<std::string>(const std::string& path, const std::string& value)
{
    rapidjson::Pointer(path.c_str()).Create(document);
    if (auto* val = rapidjson::Pointer(path.c_str()).Get(document); val)
    {
        val->SetString(value.c_str(), value.size(), document.GetAllocator());
        return;
    }

    throw std::out_of_range("Error setting string under " + path);
}

template <>
void json_reader::set<json_reader>(const std::string& path, const json_reader& value)
{
    rapidjson::Pointer(path.c_str()).Create(document);
    if (auto* val = rapidjson::Pointer(path.c_str()).Get(document); val)
    {
        val->CopyFrom(value.document, document.GetAllocator());
        return;
    }

    throw std::out_of_range("Error setting object under " + path);
}

template <>
void json_reader::set<std::vector<std::string>>(const std::string& path,
                                                const std::vector<std::string>& values)
{
    rapidjson::Pointer(path.c_str()).Create(document);
    if (auto* val = rapidjson::Pointer(path.c_str()).Get(document); val)
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

    throw std::out_of_range("Error setting object under " + path);
}

bool json_reader::is_present(const std::string& path)
{
    const rapidjson::Pointer ptr{path.c_str()};
    return ptr.IsValid() && ptr.Get(document) != nullptr;
}

bool json_reader::is_string(const std::string& path)
{
    if (const rapidjson::Pointer ptr{path.c_str()}; ptr.IsValid())
    {
        const auto* val = ptr.Get(document);
        return val && val->GetType() == rapidjson::kStringType;
    }

    return false;
}

bool json_reader::is_number(const std::string& path)
{
    if (const rapidjson::Pointer ptr{path.c_str()}; ptr.IsValid())
    {
        const auto* val = ptr.Get(document);
        return val && val->GetType() == rapidjson::kNumberType;
    }

    return false;
}

void json_reader::erase(const std::string& path)
{
    rapidjson::Pointer(path.c_str()).Erase(document);
}

}  // namespace traffic
