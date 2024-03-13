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
        getLastTouchPosRel(&touchX, &touchY);

        std::cout << touchX << ", " << touchY << std::endl;

        m_surface->drawRoundRect(static_cast<int16_t>(touchX - 5), static_cast<int16_t>(touchY - 5), 10, 10, 10, graphics::packRGB565(255, 0, 0));

        // Mark dirty
        m_isDrawn = false;
    }
} // gui::elements