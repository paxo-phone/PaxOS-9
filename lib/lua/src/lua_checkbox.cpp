#include "lua_checkbox.hpp"

LuaCheckbox::LuaCheckbox(LuaWidget* parent, int x, int y)
{
    widget = new Checkbox(x, y);
    init(widget, parent);
}
