//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "Surface.hpp"

namespace graphics
{
    Surface::Surface(const uint16_t width, const uint16_t height)
    {
        m_sprite.setColorDepth(16);

        m_sprite.createSprite(width, height);
    }

    uint16_t Surface::getWidth() const
    {
        return m_sprite.width();
    }

    uint16_t Surface::getHeight() const
    {
        return m_sprite.height();
    }

    void Surface::clear()
    {
        m_sprite.clear();
    }

    void Surface::setColor(const uint8_t r, const uint8_t g, const uint8_t b)
    {
        m_sprite.setColor(r, g, b);
    }

    void Surface::fillRect(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h)
    {
        m_sprite.fillRect(x, y, w, h);
    }

    void Surface::drawRect(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h)
    {
        m_sprite.drawRect(x, y, w, h);
    }
} // graphics