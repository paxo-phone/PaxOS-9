//
// Created by Charles on 11/09/2024.
//

#ifndef CORELIBRARY_HPP
#define CORELIBRARY_HPP

#include <LuaLibrary.hpp>
#include <string>

namespace paxolua::lib {
    class CoreLibrary final : public LuaLibrary {
    protected:
        void load(LuaEnvironment *env) override;

        void update(LuaEnvironment *env) override;

    private:
        static void print(const std::string &message);
    };
} // paxolua::lib

#endif //CORELIBRARY_HPP
