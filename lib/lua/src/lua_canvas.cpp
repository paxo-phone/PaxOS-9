#include "lua_canvas.hpp"
#include "lua_file.hpp"

#include <libsystem.hpp>
#include <LuaEnvironment.hpp>

LuaCanvas::LuaCanvas(LuaWidget* parent, int x, int y, int width, int height, LuaFile* lua)
{
    widget = new Canvas(x, y, width, height);
    this->lua = lua;
    init(widget, parent);
}

LuaCanvas::LuaCanvas(LuaWidget *parent, const int x, const int y, const int width, const int height, paxolua::LuaEnvironment *env) {
    m_env = env;

    widget = new Canvas(x, y, width, height);
    init(widget, parent);
}

sol::table LuaCanvas::getTouch()
{
    if (lua != nullptr) {
        int16_t x = gui::ElementBase::touchX, y = gui::ElementBase::touchY;

        sol::table result = lua->lua.create_table();
        result.set(1, x - widget->getAbsoluteX());
        result.set(2, y - widget->getAbsoluteY());

        return result;
    }

    if (m_env != nullptr) {
        const int16_t x = gui::ElementBase::touchX;
        const int16_t y = gui::ElementBase::touchY;

        sol::table output = m_env->getLuaState().create_table();

        output.set(1, x - widget->getAbsoluteX());
        output.set(2, y - widget->getAbsoluteY());

        output.set("x", x - widget->getAbsoluteX());
        output.set("y", y - widget->getAbsoluteY());

        return output;
    }

    throw new libsystem::exceptions::RuntimeError("Invalid state.");
}

void LuaCanvas::specificUpdate()
{
    if(widget->isFocused(true) && onTouchFunc)
    {
        onTouchFunc(getTouch());
    }
}