//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "Surface.hpp"

namespace graphics
{
    uint16_t Surface::getPos(const uint16_t x, const uint16_t y) const
    {
        return y * m_width + x;
    }

    Surface::Surface(const uint16_t width, const uint16_t height)
    {
        m_width = width;
        m_heigth = height;
        m_buffer = std::vector<uint16_t>(width * height);
    }

    uint16_t Surface::getWidth() const
    {
        return m_width;
    }

    uint16_t Surface::getHeight() const
    {
        return m_heigth;
    }

    const std::vector<uint16_t>* Surface::getBuffer() const
    {
        return &m_buffer;
    }

    uint16_t Surface::getPixel(const uint16_t x, const uint16_t y) const
    {
        return m_buffer[getPos(x, y)];
    }

    void Surface::setPixel(const uint16_t x, const uint16_t y, const uint16_t color)
    {
        m_buffer[getPos(x, y)] = color;
    }
} // graphics