//
// Created by Charles on 11/09/2024.
//

#include "CoreLibrary.hpp"

#include <libsystem.hpp>
#include <LuaEnvironment.hpp>
#include <SOL2/sol.hpp>

std::string getPointerAddress(const void *ptr) {
    std::stringstream stringStream;

    stringStream << ptr;

    return stringStream.str();
}

// https://www.grepper.com/answers/33518/lua+dump+table?ucard=1
std::string dump(const sol::object &obj) { // NOLINT(*-no-recursion)
    if (obj.is<sol::table>()) {
        const auto table = obj.as<sol::table>();

        std::string output = "{ ";

        for (const auto &[key, value] : table) {
            std::string keyString;

            if (key.is<std::string>()) {
                keyString = key.as<std::string>();
            } else {
                keyString = std::to_string(key.as<int>());
            }

            output += "[" + keyString + "] = " + dump(value) + ", ";
        }

        return output + "}";
    }

    // Value to string implementations
    if (obj.is<sol::function>()) {
        return "function: " + getPointerAddress(obj.as<sol::function>().pointer());
    }

    return obj.as<std::string>();
}

namespace paxolua::lib {
    void CoreLibrary::load(LuaEnvironment *env) {
        sol::state &lua = env->getLuaState();

        lua["print"] = [&](const sol::object &obj) {
            print(obj.as<std::string>());
        };

        // Capture "env" by value or the pointer will be lost !
        lua["require"] = [&, env](const std::string &filename) -> sol::object {
            const storage::Path libraryPath = helper::convertPath(env, filename + ".lua");

            if (!libraryPath.exists()) {
                throw libsystem::exceptions::RuntimeError("Unable to load library: " + libraryPath.str() + ".");
            }

            sol::load_result file = lua.load_file(libraryPath.str());
            if (!file.valid()) {
                const sol::error error = file;
                throw libsystem::exceptions::RuntimeError("Error loading lua file '" + filename + "': " + error.what() + ".");
            }

            return file();
        };

        lua["table"]["dump"] = [&](const sol::table &table) {
            print(dump(table));
        };
    }

    void CoreLibrary::update(LuaEnvironment *env) {

    }

    void CoreLibrary::print(const std::string &message) {
        libsystem::log("[PaxoLua] " + message);
    }
} // paxolua::lib