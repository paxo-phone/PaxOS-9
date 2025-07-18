#include "lua_label.hpp"

#include <libsystem.hpp>

LuaLabel::LuaLabel(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = std::make_shared<Label>(x, y, width, height);
    init(widget, parent);
    widget->setBackgroundColor(
        widget->parent_ == nullptr ? COLOR_WHITE : widget->parent_->getBackgroundColor()
    );
}

void LuaLabel::setTextColor(const color_t color) const
{
    widget->setTextColor(color);
}

void LuaLabel::setTextColorRGB(const uint8_t r, const uint8_t g, const uint8_t b) const
{
    widget->setTextColor(graphics::packRGB565(r, g, b));
}
