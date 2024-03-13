#include "lua_switch.hpp"

LuaSwitch::LuaSwitch(LuaWidget* parent, int x, int y)
{
    widget = new Switch(x, y);
    init(widget, parent);
}