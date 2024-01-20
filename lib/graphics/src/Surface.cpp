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

    Surface::~Surface()
    {
        m_sprite.deleteSprite();
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

    void Surface::drawLine(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2)
    {
        m_sprite.drawLine(
            x1 / graphics::getRenderScale(),
            y1 / graphics::getRenderScale(),
            x2 / graphics::getRenderScale(),
            y2 / graphics::getRenderScale()
        );
    }

    void Surface::drawImage(const Image& image, const int16_t x, const int16_t y)
    {
        switch (image.getType())
        {
        case BMP:
            m_sprite.drawBmp(image.getData(), image.getSize(), 0, 0, static_cast<int32_t>(image.getWidth()), static_cast<int32_t>(image.getHeight()));
            break;
        case PNG:
            m_sprite.drawPng(image.getData(), image.getSize(), 0, 0, static_cast<int32_t>(image.getWidth()), static_cast<int32_t>(image.getHeight()));
            break;
        case JPG:
            m_sprite.drawJpg(image.getData(), image.getSize(), 0, 0, static_cast<int32_t>(image.getWidth()), static_cast<int32_t>(image.getHeight()));
            break;
        }
    }

} // graphics