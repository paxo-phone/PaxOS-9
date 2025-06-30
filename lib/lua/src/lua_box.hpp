#ifndef LUA_BOX_MODULE
#define LUA_BOX_MODULE

#include "lua_widget.hpp"

class LuaBox : public LuaWidget
{
  public:
    LuaBox(LuaWidget* parent, int x, int y, int width, int height);

    void setRadius(uint16_t radius);

    Box* widget = nullptr;
};

#endif // LUA_BOX_MODULE
