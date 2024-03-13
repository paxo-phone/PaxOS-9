#include "lua_list.hpp"

LuaVerticalList::LuaVerticalList(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new VerticalList(x, y, width, height);
    init(widget, parent);
}

LuaHorizontalList::LuaHorizontalList(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new HorizontalList(x, y, width, height);
    init(widget, parent);
}