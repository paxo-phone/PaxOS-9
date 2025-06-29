//
// Created by Charles on 11/09/2024.
//

#ifndef LUA_SYSTEM_HPP
#define LUA_SYSTEM_HPP

#include <string>

namespace paxolua::system::config
{
    bool getBool(const std::string& key);
    int getInt(const std::string& key);
    float getFloat(const std::string& key);
    std::string getString(const std::string& key);

    void setBool(const std::string& key, bool value);
    void setInt(const std::string& key, int value);
    void setFloat(const std::string& key, float value);
    void setString(const std::string& key, const std::string& value);

    // TODO: Make "write()" auto.
    void write();
} // namespace paxolua::system::config

#endif // LUA_SYSTEM_HPP
