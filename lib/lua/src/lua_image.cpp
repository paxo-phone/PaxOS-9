#include "lua_image.hpp"

LuaImage::LuaImage(LuaWidget* parent, storage::Path path, int x, int y, int width, int height, color_t background)
{
    std::cout << path.str() << std::endl;
    widget = new Image(path, x, y, width, height, background);
    init(widget, parent);
    std::cout << "c:" << background << std::endl;
    widget->load(background);
}