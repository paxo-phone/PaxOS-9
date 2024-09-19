//
// Created by Charles on 11/09/2024.
//

#include "LuaEnvironment.hpp"

#include <filestream.hpp>
#include <json.hpp>
#include <libsystem.hpp>
#include <standby.hpp>

#include "lib/core/CoreLibrary.hpp"
#include "lib/gui/GUILibrary.hpp"
#include "lib/old/OldLibrary.hpp"


void * luaCustomAllocator(void *ud, void *ptr, size_t osize, size_t nsize) {
    if (nsize == 0) {
        // Free the block
        if (ptr != nullptr) {
            free(ptr);
        }
        return nullptr;
    }

    // Allocate or resize the block
#ifdef ESP_PLATFORM
    return ps_realloc(ptr, nsize);
#else
    return realloc(ptr, nsize);
#endif
}

bool jsonArrayHas(nlohmann::json array, const std::string &value) {
    // for (const nlohmann::json &e : array) {
    //     libsystem::log("=> " + e.get<std::string>());
    // }

    return std::find(
        array.begin(),
        array.end(),
        value
    ) != array.end();
}

namespace paxolua {
    LuaEnvironment::LuaEnvironment(const storage::Path &workingDirectory) {
        m_workingDirectory = workingDirectory;

        // Set custom allocator for what ?
        lua_setallocf(m_lua.lua_state(), luaCustomAllocator, nullptr);

        m_lua.open_libraries(
            sol::lib::base,
            sol::lib::math,
            sol::lib::table,
            sol::lib::string
        );

        m_paxoTable = m_lua["paxo"].get_or_create<sol::table>();

        // Load base library
        loadLibrary(std::make_shared<lib::CoreLibrary>());
    }

    LuaEnvironment::~LuaEnvironment() = default;

    storage::Path LuaEnvironment::getWorkingDirectory() const {
        return m_workingDirectory;
    }

    sol::state & LuaEnvironment::getLuaState() {
        return m_lua;
    }

    sol::table LuaEnvironment::getPaxoNamespace() {
        return m_lua["paxo"]; // No check should be necessary
    }

    void LuaEnvironment::run() {
        // Get and check load function
        const sol::protected_function paxoLoad = m_paxoTable["load"];
        if (!paxoLoad.valid()) {
            throw libsystem::exceptions::RuntimeError("Unable to find 'paxo.load()'.");
        }

        // Get and check update function
        m_updateFunction = m_paxoTable["update"];
        if (!m_updateFunction.valid()) {
            throw libsystem::exceptions::RuntimeError("Unable to find 'paxo.update()'.");
        }

        // Run "paxo.load()"
        if (const sol::protected_function_result paxoLoadResult = paxoLoad(); !paxoLoadResult.valid()) {
            const sol::error error = paxoLoadResult;

            libsystem::log("'paxo.load()' error: " + std::string(error.what()) + ".");
        }
    }

    void LuaEnvironment::loadFile(const storage::Path &filename) {
        if (!filename.exists()) {
            throw libsystem::exceptions::InvalidArgument("Invalid filename, file does not exist.");
        }

        auto fileStream = storage::FileStream(filename.str(), storage::READ);

        // Load file
        sol::load_result chunk = m_lua.load(fileStream.read());
        if (!chunk.valid()) {
            const sol::error error = chunk;
            throw libsystem::exceptions::RuntimeError("Unable to load file '" + filename.str() + "': " + error.what() + ".");
        }

        fileStream.close();

        // Run file
        if (sol::protected_function_result chunkResult = chunk(); !chunkResult.valid()) {
            const sol::error error = chunkResult;
            throw libsystem::exceptions::RuntimeError("Unable to run file '" + filename.str() + "': " + error.what() + ".");
        }
    }

    void LuaEnvironment::loadLibrary(const std::shared_ptr<LuaLibrary> &library) {
        library->load(this);

        m_libraries.push_back(library);
    }

    void LuaEnvironment::update() {
        // Update every library
        for (const std::shared_ptr<LuaLibrary> &library : m_libraries) {
            library->update(this);
        }

        // Call "paxo.update()"
        if (const sol::protected_function_result updateResult = m_updateFunction(); !updateResult.valid()) {
            std::cerr << "[PaxoLua] Update error." << std::endl;
        }
    }

    void helper::applyManifest(LuaEnvironment *env, const storage::Path &manifest) {
        auto fileStream = storage::FileStream(manifest.str(), storage::READ);

        std::string manifestData = fileStream.read();

        fileStream.close();

        if (!nlohmann::json::accept(manifestData)) {
            throw libsystem::exceptions::RuntimeError("Invalid manifest.");
        }

        const nlohmann::json manifestJson = nlohmann::json::parse(manifestData);

        manifestData.clear();

        // TODO: Use AppManifest class ?

        const nlohmann::json& permissions = manifestJson["permissions"];

        if (jsonArrayHas(permissions, "gui")) {
            libsystem::log("Loading GUI.");
            env->loadLibrary(std::make_shared<lib::GUILibrary>());
        }
        if (jsonArrayHas(permissions, "_")) {
            env->loadLibrary(std::make_shared<lib::GUILibrary>());
        }
        // ...

        if (manifestJson.contains("compatibility_mode")) {
            // ReSharper disable once CppTooWideScopeInitStatement
            const std::string compatibilityMode = manifestJson["compatibility_mode"];

            if (compatibilityMode == "old_lua") {
                libsystem::log("Using 'old_lua' compatibility mode.");
                env->loadLibrary(std::make_shared<lib::OldLibrary>());
            }
        }
    }

    storage::Path helper::convertPath(const LuaEnvironment *env, const storage::Path &path) {
        if (path.m_steps[0] == "/") {
            throw libsystem::exceptions::RuntimeError("Illegal storage access.");
        }

        // Not used.
        if (path.m_steps[0] == "/") {
            return path;
        }

        return env->getWorkingDirectory() / path;
    }
} // paxolua