//
// Created by Charles on 11/09/2024.
//

#include "libsystem.hpp"

#include <libsystem.hpp>

void paxolua::system::load(const sol::state &lua) {
    auto paxo = lua["paxo"].get<sol::table>();

    // paxo.system
    auto system = paxo["system"].get_or_create<sol::table>(sol::new_table());

    // paxo.system.config
    auto systemConfig = system["config"].get_or_create<sol::table>(sol::new_table());

    // paxo.system.config.get()
    systemConfig.set_function("get", sol::overload(
        &config::getBool,
        &config::getFloat,
        &config::getString
    ));

    // paxo.system.config.set()
    systemConfig.set_function("set", sol::overload(
        &config::setBool,
        &config::setFloat,
        &config::setString
    ));

    // paxo.system.config.write() / paxo.system.config.save()
    systemConfig.set_function("write", &config::write);
    systemConfig.set_function("save", &config::write);
}

bool paxolua::system::config::getBool(const std::string &key) {
    libsystem::log("getBool: " + key);
    return libsystem::getSystemConfig().get<bool>(key);
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
