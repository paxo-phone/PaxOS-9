#include "lua_radio.hpp"

LuaRadio::LuaRadio(LuaWidget* parent, int x, int y)
{
    widget = std::make_shared<Radio>(x, y);
    init(widget, parent);
}
