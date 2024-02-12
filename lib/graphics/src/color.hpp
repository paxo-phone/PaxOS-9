//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#ifndef COLOR_HPP
#define COLOR_HPP

#include <cstdint>
typedef uint16_t color_t;

namespace graphics
{
    color_t packRGB565(uint8_t r, uint8_t g, uint8_t b);
    void unpackRGB565(color_t rgb, uint8_t *r, uint8_t *g, uint8_t *b);
}

#endif //COLOR_HPP
