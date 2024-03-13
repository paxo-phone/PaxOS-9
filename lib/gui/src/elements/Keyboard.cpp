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
        m_surface->clear(COLOR_WHITE);
        m_surface->fillRect(0, 0, m_width, m_height, graphics::packRGB565(0, 0, 255));
    }

    void Keyboard::onReleased()
    {
        std::cout << "KEYBOARD CLICK !" << touchX << touchX << std::endl;
    }
} // gui::elements