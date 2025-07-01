//
// Created by Charles on 17/01/2024.
//

#include "Box.hpp"

#include <Surface.hpp>
#include <cstdio>
#include <graphics.hpp>
#include <iostream>

namespace gui::elements
{
    Box::Box(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height)
    {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
        pressed = false;
    }

    Box::~Box() = default;

    void Box::render()
    {
        if (borderRadius_ > width_ / 2 || borderRadius_ > height_ / 2)
            borderRadius_ = std::min(width_ / 2, height_ / 2);
        surface_->fillRect(0, 0, width_, height_, COLOR_WHITE);
        if (borderSize_ || borderRadius_ || backgroundColor_ != COLOR_WHITE)
        {
            surface_->fillRoundRectWithBorder(
                0,
                0,
                width_,
                height_,
                borderRadius_,
                borderSize_,
                backgroundColor_,
                borderColor_
            );
        }
    }

    void Box::postRender()
    {
        if (pressed)
            surface_->drawRoundRect(0, 0, width_, height_, 3, COLOR_GREY);
    }

    void Box::onClick()
    {
        pressed = true;

        localGraphicalUpdate();
    }

    void Box::onNotClicked()
    {
        std::cout << "Box::onNotClicked()" << std::endl;
        pressed = false;

        localGraphicalUpdate();
    }

} // namespace gui::elements
