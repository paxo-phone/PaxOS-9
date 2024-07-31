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
#define COLOR_SUCCESS 0x2D28
#define COLOR_WARNING 0xFE00
#define COLOR_ERROR 0xD9A8


// #define COLOR_WHITE 0xFFFF
// #define COLOR_BLACK 0x0000
// #define COLOR_GREY 0x8410

#define COLOR_WHITE         graphics::packRGB565(255, 255, 255)
#define COLOR_BLACK         graphics::packRGB565(0, 0, 0)
#define COLOR_RED           graphics::packRGB565(255, 0, 0)
#define COLOR_GREEN         graphics::packRGB565(0, 128, 0)
#define COLOR_BLUE          graphics::packRGB565(0, 0, 255)

#define COLOR_YELLOW        graphics::packRGB565(255, 255, 0)
#define COLOR_GREY          graphics::packRGB565(128, 128, 128)
#define COLOR_MAGENTA       graphics::packRGB565(255, 0, 255)
#define COLOR_CYAN          graphics::packRGB565(0, 255, 255)
#define COLOR_VIOLET        graphics::packRGB565(128, 0, 128)
#define COLOR_ORANGE        graphics::packRGB565(255, 165, 0)
#define COLOR_PINK          graphics::packRGB565(255,192,203)

#define COLOR_LIGHT_GREEN   graphics::packRGB565(0, 255, 0)
#define COLOR_LIGHT_BLUE    graphics::packRGB565(173,216,230)
#define COLOR_LIGHT_GREY    graphics::packRGB565(211,211,211)

#endif //COLOR_HPP
