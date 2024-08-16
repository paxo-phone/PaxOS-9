#include "lua_json.hpp"

LuaJson::LuaJson(std::string data)
    : json(*(new nlohmann::json)),
      allocated(true)
{
    //delete &json;   // si quelqu'un arrive a faire mieux :/

    if (nlohmann::json::accept(data))
        json = nlohmann::json::parse(data);
    else
        json = nlohmann::json::parse("{}");
}

LuaJson::LuaJson(nlohmann::json& other)
    : json(other)
{}

std::string LuaJson::get()
{
    return json.dump();
}

LuaJson LuaJson::op(std::string key)
{
    return LuaJson(json[key]);
}

LuaJson LuaJson::at(int position)
{
    return LuaJson(json.at(position));
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
    return json[key].get<int>();
}

double LuaJson::get_double(std::string key)
{
    return json[key].get<double>();
}

bool LuaJson::get_bool(std::string key)
{
    return json[key].get<bool>();
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