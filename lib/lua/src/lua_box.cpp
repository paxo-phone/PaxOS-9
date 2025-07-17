#include "lua_box.hpp"

LuaBox::LuaBox(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new Box(x, y, width, height);
    init(widget, parent);
    widget->setBackgroundColor(
        widget->m_parent == nullptr ? COLOR_WHITE : widget->m_parent->getBackgroundColor()
    );
}

void LuaBox::setRadius(uint16_t radius)
{
    this->widget->setRadius(radius);
}
