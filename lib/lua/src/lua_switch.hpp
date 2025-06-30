#ifndef LUA_SWITCH_MODULE
#define LUA_SWITCH_MODULE

#include "lua_widget.hpp"

class LuaSwitch : public LuaWidget
{
  public:
    LuaSwitch(LuaWidget* parent, int x, int y, LuaGui* gui);

    void setState(bool state)
    {
        widget->setState(state);
    }

    bool getState()
    {
        return widget->getState();
    }

    void onChange(sol::protected_function func)
    {
        onChangeFunc = func;
    }

    Switch* widget = nullptr;
    sol::protected_function onChangeFunc;
};

#endif
