//
// Created by Charles on 11/09/2024.
//

#ifndef OLDLIBRARY_HPP
#define OLDLIBRARY_HPP

#include <LuaLibrary.hpp>
#include <lua_file.hpp>

namespace paxolua::lib {
    class OldLibrary final : public LuaLibrary {
    protected:
        void load(LuaEnvironment *env) override;

        void update(LuaEnvironment *env) override;
    };
} // paxolua::lib

#endif //OLDLIBRARY_HPP
