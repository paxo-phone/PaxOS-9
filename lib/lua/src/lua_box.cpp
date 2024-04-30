#include "lua_box.hpp"

LuaBox::LuaBox(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new Box(x, y, width, height);
    init(widget, parent);
}

void LuaBox::setRadius(uint16_t radius)
{
    this->widget->setRadius(radius);
}