#include "lua_image.hpp"

LuaImage::LuaImage(LuaWidget* parent, storage::Path path, int x, int y, int width, int height, color_t background)
{
    widget = new Image(path, x, y, width, height, background);
    init(widget, parent);
}

void LuaImage::setTransparentColor (color_t color) {
        widget->setTransparentColor(color);
}

/*
void LuaImage::setTransparency(bool _transparent) {
        widget->setTransparency(_transparent);
}
*/

