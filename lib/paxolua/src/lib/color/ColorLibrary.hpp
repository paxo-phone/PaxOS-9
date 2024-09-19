//
// Created by Charles on 11/09/2024.
//

#ifndef BASELIBRARY_HPP
#define BASELIBRARY_HPP

#include <LuaLibrary.hpp>

namespace paxolua::lib {
    class ColorLibrary final : public LuaLibrary {
    protected:
        void load(LuaEnvironment *env) override;

        void update(LuaEnvironment *env) override;
    };
} // paxolua::lib

#endif //BASELIBRARY_HPP
