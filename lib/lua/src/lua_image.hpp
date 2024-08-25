#ifndef LUA_IMAGE_MODULE
#define LUA_IMAGE_MODULE

#include "lua_widget.hpp"

class LuaImage : public LuaWidget
{
    public:
        // LuaImage(LuaWidget* parent, storage::Path path, int x, int y, int width, int height);
        LuaImage(LuaWidget* parent, storage::Path path, int x, int y, int width, int height, color_t background = COLOR_WHITE);
        void setTransparentColor (color_t color);
//        void setTransparency(bool _transparent);

        Image* widget = nullptr;
};

#endif