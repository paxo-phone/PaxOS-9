#include "lua_canvas.hpp"

LuaCanvas::LuaCanvas(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new Canvas(x, y, width, height);
    init(widget, parent);
}