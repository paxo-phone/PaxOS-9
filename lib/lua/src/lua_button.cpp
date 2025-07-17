#include "lua_button.hpp"

LuaButton::LuaButton(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new Button(x, y, width, height);
    init(widget, parent);
}
