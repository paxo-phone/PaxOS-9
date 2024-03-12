#include "lua_image.hpp"

LuaImage::LuaImage(LuaWidget* parent, storage::Path path, int x, int y, int width, int height)
{
    widget = new Image(path, x, y, width, height);
    init(widget, parent, x, y, width, height);
    widget->load();
}