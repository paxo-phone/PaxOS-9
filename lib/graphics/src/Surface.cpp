//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "Surface.hpp"

namespace graphics
{
    uint16_t Surface::getPos(const uint16_t x, const uint16_t y) const
    {
        return y * width + x;
    }

    Surface::Surface(const uint16_t width, const uint16_t height)
    {
        this->width = width;
        this->heigth = height;
        buffer = std::vector<uint16_t>(width * height);
    }

    uint16_t Surface::getWidth() const
    {
        return width;
    }

    uint16_t Surface::getHeight() const
    {
        return heigth;
    }

    const std::vector<uint16_t>* Surface::getBuffer() const
    {
        return &buffer;
    }

    uint16_t Surface::getPixel(const uint16_t x, const uint16_t y) const
    {
        return buffer[getPos(x, y)];
    }

    void Surface::setPixel(const uint16_t x, const uint16_t y, const uint16_t color)
    {
        buffer[getPos(x, y)] = color;
    }
} // graphics