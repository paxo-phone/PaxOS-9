#include "lua_label.hpp"

#include <libsystem.hpp>

LuaLabel::LuaLabel(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new Label(x, y, width, height);
    init(widget, parent);
}

void LuaLabel::setTextColor(const color_t color) const {
    libsystem::log("Color: " + std::to_string(color));

    widget->setTextColor(color);
}

void LuaLabel::setTextColorRGB(const uint8_t r, const uint8_t g, const uint8_t b) const {
    libsystem::log("Color RGB: " + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b));

    widget->setTextColor(graphics::packRGB565(r, g, b));
}

