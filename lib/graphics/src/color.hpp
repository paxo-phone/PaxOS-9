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

#define COLOR_DARK 0x0000
#define COLOR_LIGHT 0xF7BE
#define COLOR_WHITE 0xFFFF
#define COLOR_SUCCESS 0x2D28
#define COLOR_WARNING 0xFE00
#define COLOR_ERROR 0xD9A8
#define COLOR_GREY 0x8410

#endif //COLOR_HPP
