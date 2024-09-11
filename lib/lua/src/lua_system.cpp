//
// Created by Charles on 11/09/2024.
//

#include "lua_system.hpp"

#include <libsystem.hpp>

bool paxolua::system::config::getBool(const std::string &key) {
    libsystem::log("getBool: " + key);
    return libsystem::getSystemConfig().get<bool>(key);
}

int paxolua::system::config::getInt(const std::string &key) {
    libsystem::log("getInt: " + key);
    return libsystem::getSystemConfig().get<int>(key);
}

float paxolua::system::config::getFloat(const std::string &key) {
    libsystem::log("getFloat: " + key);
    return libsystem::getSystemConfig().get<float>(key);
}

std::string paxolua::system::config::getString(const std::string &key) {
    libsystem::log("getString: " + key);
    return libsystem::getSystemConfig().get<std::string>(key);
}

void paxolua::system::config::setBool(const std::string &key, const bool value) {
    libsystem::log("setBool: " + key + ", " + std::to_string(value));
    libsystem::getSystemConfig().set<bool>(key, value);
}

void paxolua::system::config::setInt(const std::string &key, const int value) {
    libsystem::log("setInt: " + key + ", " + std::to_string(value));
    libsystem::getSystemConfig().set<int>(key, value);
}

void paxolua::system::config::setFloat(const std::string &key, const float value) {
    libsystem::log("setFloat: " + key + ", " + std::to_string(value));
    libsystem::getSystemConfig().set<float>(key, value);
}

void paxolua::system::config::setString(const std::string &key, const std::string &value) {
    libsystem::log("setString: " + key + ", " + value);
    libsystem::getSystemConfig().set<std::string>(key, value);
}

void paxolua::system::config::write() {
    libsystem::log("write");
    libsystem::getSystemConfig().write();
}
