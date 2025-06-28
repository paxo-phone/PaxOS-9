#include "lua_radio.hpp"

LuaRadio::LuaRadio(LuaWidget* parent, int x, int y)
{
    widget = new Radio(x, y);
    init(widget, parent);
}
