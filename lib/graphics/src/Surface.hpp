//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#ifndef SURFACE_HPP
#define SURFACE_HPP

#include <cstdint>
#include <vector>

namespace graphics
{
    class Surface
    {
    private:
        uint16_t width;
        uint16_t heigth;
        std::vector<uint16_t> buffer;

        uint16_t getPos(uint16_t x, uint16_t y) const;

    public:
        Surface(uint16_t width, uint16_t height);

        uint16_t getWidth() const;
        uint16_t getHeight() const;
        const std::vector<uint16_t> * getBuffer() const;

        uint16_t getPixel(uint16_t x, uint16_t y) const;
        void setPixel(uint16_t x, uint16_t y, uint16_t color);
    };
} // graphics

#endif //SURFACE_HPP
