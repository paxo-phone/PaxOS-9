#ifndef LUA_EVENT_MODULE
#define LUA_EVENT_MODULE

#include "SOL2/sol.hpp"

class LuaFile;

class LuaTimeInterval
{
    public:
    LuaTimeInterval(LuaFile* lua, sol::protected_function func, uint32_t interval);
    int getId();
    void call();
    ~LuaTimeInterval();

    private:
    LuaFile* lua;
    sol::protected_function func;
    uint32_t interval;
    int id;
};

class LuaTimeEvent
{
    public:
        uint32_t addEventListener(LuaFile* lua, sol::protected_function condition, sol::protected_function callback);
        void call();
    private:
        int id;
        sol::protected_function condition;
        sol::protected_function callback;

};

class LuaTimeTimeout
{
    public:
    LuaTimeTimeout(LuaFile* lua, sol::protected_function func, uint32_t timeout);
    int getId();
    void call();
    ~LuaTimeTimeout();

    bool done = false;

    private:
    LuaFile* lua;
    sol::protected_function func;
    uint32_t timeout;
    int id;
};

class LuaTime
{
    public:
    LuaTime(LuaFile* lua);
    ~LuaTime();
    void update();
    
    uint32_t monotonic();
    sol::table get(std::string format);
    int setInterval(sol::protected_function func, uint32_t interval);
    int setTimeout(sol::protected_function func, uint32_t timeout);

    void removeInterval(int id);
    void removeTimeout(int id);

    private:
    LuaFile* lua = nullptr;
    uint32_t timerFromStart = 0;

    std::vector<LuaTimeInterval*> intervals;
    std::vector<LuaTimeTimeout*> timeouts;
};

#endif