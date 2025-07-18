#include "lua_switch.hpp"

#include "lua_file.hpp"
#include "lua_gui.hpp"

LuaSwitch::LuaSwitch(LuaWidget* parent, int x, int y, LuaGui* gui)
{
    widget = std::make_shared<Switch>(x, y);
    init(widget, parent);

    auto my_lambda = [](int x, int y) -> int
    {
        return x + y;
    };

    this->gui = gui;

    sol::protected_function lua_function = this->gui->lua->lua["donothing"];

    onClick(lua_function);
}
