#include "lua_canvas.hpp"

#include "lua_file.hpp"

#include <graphics.hpp>

LuaCanvas::LuaCanvas(LuaWidget* parent, int x, int y, int width, int height, LuaFile* lua)
{
    widget = new Canvas(x, y, width, height);
    this->lua = lua;
    init(widget, parent);
}

sol::table LuaCanvas::getTouch()
{
    int16_t x = gui::ElementBase::touchX, y = gui::ElementBase::touchY;

    sol::table result = lua->lua.create_table();
    result.set(1, x - widget->getAbsoluteX());
    result.set(2, y - widget->getAbsoluteY());

    return result;
}

void LuaCanvas::specificUpdate()
{
    if (widget->isFocused(true) && onTouchFunc)
        onTouchFunc(getTouch());
}
