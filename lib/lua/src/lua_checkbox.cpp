#include "lua_checkbox.hpp"

LuaCheckbox::LuaCheckbox(LuaWidget* parent, int x, int y)
{
    widget = std::make_shared<Checkbox>(x, y);
    init(widget, parent);
}
