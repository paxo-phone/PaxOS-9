#include "lua_label.hpp"

LuaLabel::LuaLabel(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new Label(x, y, width, height);
    init(widget, parent);
}
