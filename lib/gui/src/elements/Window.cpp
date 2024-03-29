//
// Created by Gabriel on 12/02/2024.
//

#include "Window.hpp"

#include <cstdio>
#include <graphics.hpp>
#include <Surface.hpp>

namespace gui::elements {
    Window::Window()
    {
        m_x = 0;
        m_y = 0;
        m_width = graphics::getScreenWidth();
        m_height = graphics::getScreenHeight();
        m_backgroundColor = COLOR_WHITE;
    }

    Window::~Window() = default;

    void Window::render()
    {
        m_surface->fillRect(0, 0, m_width, m_height, m_backgroundColor);
    }
} // gui::elements