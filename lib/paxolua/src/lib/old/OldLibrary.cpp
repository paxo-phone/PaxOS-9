//
// Created by Charles on 11/09/2024.
//

#include "OldLibrary.hpp"

#include <libsystem.hpp>
#include <LuaEnvironment.hpp>

namespace paxolua::lib {
    void OldLibrary::load(LuaEnvironment *env) {
        env->loadFile(storage::Path("/system/paxolua/compatibility_wrapper.lua"));
    }

    void OldLibrary::update(LuaEnvironment *env) {

    }
} // paxolua::lib