//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "Surface.hpp"

#include <graphics.hpp>

namespace graphics
{
    Surface::Surface(const uint16_t width, const uint16_t height)
    {
        m_sprite.setColorDepth(16);

        m_sprite.createSprite(width / graphics::getRenderScale(), height / graphics::getRenderScale());
    }

    uint16_t Surface::getWidth() const
    {
        return m_sprite.width();
    }

    uint16_t Surface::getHeight() const
    {
        return m_sprite.height();
    }

    void Surface::pushSurface(Surface *surface, const int16_t x, const int16_t y)
    {
        surface->m_sprite.pushSprite(
            &m_sprite,
            x / graphics::getRenderScale(),
            y / graphics::getRenderScale()
        );
    }

    void Surface::clear(const uint8_t r, const uint8_t g, const uint8_t b)
    {
        m_sprite.clear(lgfx::color565(r, g, b));
    }

    void Surface::clear()
    {
        m_sprite.clear();
    }

    void Surface::setColor(const uint8_t r, const uint8_t g, const uint8_t b)
    {
        m_sprite.setColor(r, g, b);
    }

    void Surface::fillRect(const int16_t x, const int16_t y, const uint16_t w, const uint16_t h)
    {
        m_sprite.fillRect(
            x / graphics::getRenderScale(),
            y / graphics::getRenderScale(),
            w / graphics::getRenderScale(),
            h / graphics::getRenderScale()
        );
    }

    void Surface::drawRect(const int16_t x, const int16_t y, const uint16_t w, const uint16_t h)
    {
        m_sprite.drawRect(
            x / graphics::getRenderScale(),
            y / graphics::getRenderScale(),
            w / graphics::getRenderScale(),
            h / graphics::getRenderScale()
        );
    }
} // graphics