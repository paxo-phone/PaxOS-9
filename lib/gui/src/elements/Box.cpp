//
// Created by Charles on 17/01/2024.
//

#include "Box.hpp"

#include <cstdio>
#include <graphics.hpp>
#include <Surface.hpp>

namespace gui::elements {
    Box::Box(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height)
    {
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
    }

    Box::~Box() = default;

    void Box::render()
    {
        m_surface->fillRect(0, 0, m_width, m_height, COLOR_WHITE);
        if(m_borderSize || m_borderRadius || m_backgroundColor != COLOR_WHITE)
            m_surface->fillRoundRectWithBorder(0, 0, m_width, m_height, m_borderRadius, m_borderSize, m_backgroundColor, m_borderColor);
    }

} // gui::elements