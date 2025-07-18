#include "lua_window.hpp"

#include <iostream>

LuaWindow::LuaWindow()
{
    widget = std::make_shared<Window>();
    init(widget);
}

int LuaWindow::delete_LuaWindow(LuaWindow* window)
{
    delete window;
    return 0;
}
