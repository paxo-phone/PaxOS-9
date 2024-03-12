#include "lua_input.hpp"

LuaInput::LuaInput(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new Input(x, y, width, height);    // a mettre a jour sur le paxos9
    init(widget, parent, x, y, width, height);
}