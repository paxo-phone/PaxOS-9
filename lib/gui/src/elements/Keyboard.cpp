//
// Created by Charles on 13/03/2024.
//

#include "Keyboard.hpp"

#include <iostream>
#include <graphics.hpp>
#include <Surface.hpp>

namespace gui::elements {
    Keyboard::Keyboard()
    {
        m_width = graphics::getScreenWidth();
        m_height = 200;

        m_x = 0;
        m_y = graphics::getScreenHeight() - m_height;

        m_hasEvents = true;
    }

    Keyboard::~Keyboard() = default;

    void Keyboard::render()
    {

    }

    void Keyboard::onReleased()
    {
        int16_t touchX, touchY;
        getLastTouchPos(&touchX, &touchY);

        m_surface->drawRoundRect(touchX, touchY, 10, 10, 10, graphics::packRGB565(255, 0, 0));
    }
} // gui::elements