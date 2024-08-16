#ifndef LUA_JSON
#define LUA_JSON

#include <string>
#include <json.hpp>

class LuaJson
{
public:
    LuaJson(std::string data);
    LuaJson(nlohmann::json& other);

    std::string get();
    LuaJson op(std::string key);
    void set(std::string id, std::string value);
    LuaJson at(int position);

    bool is_null();
    size_t size();
    bool has_key(std::string key);
    void remove(std::string key);


    int get_int(std::string key);
    double get_double(std::string key);
    bool get_bool(std::string key);

    void set_int(std::string key, int value);
    void set_double(std::string key, double value);
    void set_bool(std::string key, bool value);

    nlohmann::json& json;
    bool allocated = false;
};

#endif