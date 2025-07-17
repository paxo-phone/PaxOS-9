#ifndef LUA_CHECKBOX_MODULE
#define LUA_CHECKBOX_MODULE

#include "lua_widget.hpp"

class LuaCheckbox : public LuaWidget
{
  public:
    LuaCheckbox(LuaWidget* parent, int x, int y);

    void setState(bool state)
    {
        widget->setState(state);
    }

    bool getState()
    {
        return widget->getState();
    }

    Checkbox* widget = nullptr;
};

#endif
