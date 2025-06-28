//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#ifndef COLOR_HPP
#define COLOR_HPP

#include <cstdint>

/**
 * Color type, RGB565.
 */
typedef uint16_t color_t;

namespace graphics
{
    /**
     * Create a RGB565 value from 3 8-bit RGB components.
     * @param r The red component.
     * @param g The green component.
     * @param b The blue component.
     * @see graphics::constPackRGB565()
     * @return A RGB565 color.
     */
    color_t packRGB565(uint8_t r, uint8_t g, uint8_t b);

    /**
     * Extracts the red, green and blue components from a RGB565 color.
     * @param rgb The RGB565 color to unpack.
     * @param r The red component output pointer.
     * @param g The green component output pointer.
     * @param b The blue component output pointer.
     */
    void unpackRGB565(color_t rgb, uint8_t* r, uint8_t* g, uint8_t* b);

    /**
     * Create a RGB565 value from 3 8-bit RGB components.
     * This function is evaluated at compile-time.
     * @param r The red component.
     * @param g The green component.
     * @param b The blue component.
     * @return A RGB565 color.
     */
    constexpr color_t constPackRGB565(const uint8_t r, const uint8_t g, const uint8_t b)
    {
        uint16_t rgb = 0;

        rgb |= (r >> 3) << 11;
        rgb |= (g >> 2) << 5;
        rgb |= (b >> 3);

        return rgb;
    }
} // namespace graphics

#define COLOR_DARK 0x0000
#define COLOR_LIGHT 0xF7BE
#define COLOR_SUCCESS 0x2D28
#define COLOR_WARNING 0xFE00
#define COLOR_ERROR 0xD9A8

extern color_t COLOR_WHITE;
#define COLOR_BLACK graphics::constPackRGB565(0, 0, 0)
#define COLOR_RED graphics::constPackRGB565(255, 0, 0)
#define COLOR_GREEN graphics::constPackRGB565(0, 128, 0)
#define COLOR_BLUE graphics::constPackRGB565(0, 0, 255)

#define COLOR_YELLOW graphics::constPackRGB565(255, 255, 0)
#define COLOR_GREY graphics::constPackRGB565(128, 128, 128)
#define COLOR_MAGENTA graphics::constPackRGB565(255, 0, 255)
#define COLOR_CYAN graphics::constPackRGB565(0, 255, 255)
#define COLOR_VIOLET graphics::constPackRGB565(128, 0, 128)
#define COLOR_ORANGE graphics::constPackRGB565(255, 165, 0)
#define COLOR_PINK graphics::constPackRGB565(255, 192, 203)

#define COLOR_LIGHT_GREEN graphics::constPackRGB565(0, 255, 0)
#define COLOR_LIGHT_ORANGE graphics::constPackRGB565(255, 213, 128)
#define COLOR_LIGHT_BLUE graphics::constPackRGB565(173, 216, 230)
#define COLOR_LIGHT_GREY graphics::constPackRGB565(211, 211, 211)

#endif // COLOR_HPP
