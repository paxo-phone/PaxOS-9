//
// Created by Charles on 11/09/2024.
//

#include "lua_system.hpp"

#include <libsystem.hpp>

bool paxolua::system::config::getBool(const std::string& key)
{
    libsystem::log("getBool: " + key);
    return libsystem::getSystemConfig().get<bool>(key);
}

int paxolua::system::config::getInt(const std::string& key)
{ // convert any number to int
    libsystem::log("getInt: " + key);

    // Get the raw variant type
    libsystem::FileConfig::file_config_types_t value = libsystem::getSystemConfig().getRaw(key);

    // Use a visitor to return the value as int
    return std::visit(
        [](auto&& arg) -> int
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>)
            {
                return 0; // Or any default value for nullptr
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                // Attempt string to int conversion, handle errors as needed
                try
                {
                    return std::stoi(arg);
                }
                catch (const std::exception& e)
                {
                    libsystem::log("Error converting string to int: " + std::string(e.what()));
                    return 0; // Or any default value on error
                }
            }
            else if constexpr (std::is_same_v<T, std::vector<std::string>>)
            {
                // Handle vector of strings (e.g., return size, first element as
                // int, etc.) For this example, let's return the size of the
                // vector
                return arg.size();
            }
            else
            {
                return static_cast<int>(arg);
            }
        },
        value
    );
}

float paxolua::system::config::getFloat(const std::string& key)
{
    libsystem::log("getFloat: " + key);
    return libsystem::getSystemConfig().get<float>(key);
}

std::string paxolua::system::config::getString(const std::string& key)
{
    libsystem::log("getString: " + key);
    return libsystem::getSystemConfig().get<std::string>(key);
}

void paxolua::system::config::setBool(const std::string& key, const bool value)
{
    libsystem::log("setBool: " + key + ", " + std::to_string(value));
    libsystem::getSystemConfig().set<bool>(key, value);
}

void paxolua::system::config::setInt(const std::string& key, const int value)
{
    libsystem::log("setInt: " + key + ", " + std::to_string(value));
    libsystem::getSystemConfig().set<int>(key, value);
}

void paxolua::system::config::setFloat(const std::string& key, const float value)
{
    libsystem::log("setFloat: " + key + ", " + std::to_string(value));
    libsystem::getSystemConfig().set<float>(key, value);
}

void paxolua::system::config::setString(const std::string& key, const std::string& value)
{
    libsystem::log("setString: " + key + ", " + value);
    libsystem::getSystemConfig().set<std::string>(key, value);
}

void paxolua::system::config::write()
{
    libsystem::log("write");
    libsystem::getSystemConfig().write();
}
