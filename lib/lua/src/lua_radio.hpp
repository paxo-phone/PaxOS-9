#ifndef LUA_RADIO_MODULE
#define LUA_RADIO_MODULE

#include "lua_widget.hpp"

class LuaRadio : public LuaWidget
{
  public:
    LuaRadio(LuaWidget* parent, int x, int y);

    void setState(bool state)
    {
        widget->setState(state);
    }

    bool getState()
    {
        return widget->getState();
    }

    Radio* widget = nullptr;
};

#endif
