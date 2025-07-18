#ifndef LUA_WINDOW_MODULE
#define LUA_WINDOW_MODULE

#include "lua_widget.hpp"

class LuaWindow : public LuaWidget
{
  public:
    LuaWindow();
    static int delete_LuaWindow(LuaWindow* window);

    std::shared_ptr<Window> widget = nullptr;
};

#endif
