#include "lua_window.hpp"

LuaWindow::LuaWindow()  
{
    widget = new gui::elements::Window();
    init(widget);
}