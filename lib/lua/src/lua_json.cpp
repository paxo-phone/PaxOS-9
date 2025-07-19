#include "lua_json.hpp"

LuaJson::LuaJson(std::string data) : json(*(new nlohmann::json)), allocated(true)
{
    try
    {
        if (nlohmann::json::accept(data))
            json = nlohmann::json::parse(data);
        else
            json = nlohmann::json::parse("{}");
    }
    catch (const std::exception&)
    {
        // If parsing fails, set to null
        json = nullptr;
    }
}

LuaJson::LuaJson(nlohmann::json& other) : json(other) {}

std::string LuaJson::get()
{
    try
    {
        if (json.is_discarded() || json.is_null())
            return "null"; // or "" if you prefer
        return json.dump();
    }
    catch (const std::exception&)
    {
        return "";
    }
}

LuaJson LuaJson::op(std::string key)
{
    try
    {
        return LuaJson(json.at(key));
    }
    catch (const nlohmann::json::out_of_range&)
    {
        // Return a null LuaJson if key is not found
        nlohmann::json null_json = nullptr;
        return LuaJson(null_json);
    }
}

LuaJson LuaJson::op(int index)
{
    try
    {
        return LuaJson(json.at(index));
    }
    catch (const std::exception&)
    {
        nlohmann::json null_json = nullptr;
        return LuaJson(null_json);
    }
}

void LuaJson::set(std::string id, std::string value)
{
    this->json[id] = value;
}

bool LuaJson::is_null()
{
    return json.is_null();
}

size_t LuaJson::size()
{
    return json.size();
}

bool LuaJson::has_key(std::string key)
{
    return json.find(key) != json.end();
}

void LuaJson::remove(std::string key)
{
    json.erase(key);
}

int LuaJson::get_int(std::string key)
{
    try
    {
        return json.at(key).get<int>();
    }
    catch (const std::exception&)
    {
        return 0; // or another default value
    }
}

double LuaJson::get_double(std::string key)
{
    try
    {
        return json.at(key).get<double>();
    }
    catch (const std::exception&)
    {
        return 0.0;
    }
}

bool LuaJson::get_bool(std::string key)
{
    try
    {
        return json.at(key).get<bool>();
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::string LuaJson::get_string(std::string key)
{
    try
    {
        return json.at(key).get<std::string>();
    }
    catch (const std::exception&)
    {
        return "";
    }
}

void LuaJson::set_int(std::string key, int value)
{
    json[key] = value;
}

void LuaJson::set_double(std::string key, double value)
{
    json[key] = value;
}

void LuaJson::set_bool(std::string key, bool value)
{
    json[key] = value;
}
