//
// Created by Charles on 11/09/2024.
//

#ifndef LUAENVIRONMENT_HPP
#define LUAENVIRONMENT_HPP

#include <LuaLibrary.hpp>
#include <path.hpp>
#include <SOL2/sol.hpp>

namespace paxolua {
    class LuaEnvironment {
    public:
        explicit LuaEnvironment(const storage::Path &workingDirectory);
        ~LuaEnvironment();

        [[nodiscard]] storage::Path getWorkingDirectory() const;
        sol::state & getLuaState();

        sol::table getPaxoNamespace();

        /**
         * @brief Run the lua environment.
         *
         * Calls "paxo.load()".
         */
        void run();

        /**
         * @brief Load a lua file into the environment.
         *
         * @param filename The file to load.
         */
        void loadFile(const storage::Path &filename);

        /**
         * @brief Load a lua library into the environment.
         *
         * @param library The library to load.
         */
        void loadLibrary(const std::shared_ptr<LuaLibrary> &library);

        /**
         * @brief Update the lua environment.
         *
         * Calls "paxo.update()".
         */
        void update();

    private:
        sol::state m_lua;

        storage::Path m_workingDirectory;

        sol::table m_paxoTable;

        sol::protected_function m_updateFunction;

        std::vector<std::shared_ptr<LuaLibrary>> m_libraries;
    };

    namespace helper {
        void applyManifest(LuaEnvironment *env, const storage::Path &manifest);
        storage::Path convertPath(const LuaEnvironment *env, const storage::Path &path);
    }
} // paxolua

#endif //LUAENVIRONMENT_HPP
