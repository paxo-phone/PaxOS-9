#include "lua_input.hpp"

LuaInput::LuaInput(LuaWidget* parent, int x, int y)
{
    widget = new Input(x, y); // a mettre a jour sur le paxos9
    init(widget, parent);
}
