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
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
        pressed = false;
    }

    Box::~Box() = default;

    void Box::render()
    {
        if (m_borderRadius > m_width / 2 || m_borderRadius > m_height / 2)
            m_borderRadius = std::min(m_width / 2, m_height / 2);
        m_surface->fillRect(0, 0, m_width, m_height, COLOR_WHITE);
        if (m_borderSize || m_borderRadius || m_backgroundColor != COLOR_WHITE)
            m_surface->fillRoundRectWithBorder(
                0,
                0,
                m_width,
                m_height,
                m_borderRadius,
                m_borderSize,
                m_backgroundColor,
                m_borderColor
            );
    }

    void Box::postRender()
    {
        if (pressed)
            m_surface->drawRoundRect(0, 0, m_width, m_height, 3, COLOR_GREY);
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
