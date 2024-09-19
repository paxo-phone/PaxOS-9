//
// Created by Charles on 11/09/2024.
//

#ifndef LUALIBRARY_HPP
#define LUALIBRARY_HPP

namespace paxolua {
    class LuaEnvironment;

    class LuaLibrary {
        friend LuaEnvironment;

    public:
        virtual ~LuaLibrary();

    protected:
        LuaLibrary();

        virtual void load(LuaEnvironment *env) = 0;
        virtual void update(LuaEnvironment *env) = 0;
    };
} // paxolua

#endif //LUALIBRARY_HPP
