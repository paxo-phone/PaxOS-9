//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "color.hpp"

color_t COLOR_WHITE = 0xFFFF;

color_t graphics::packRGB565(const uint8_t r, const uint8_t g, const uint8_t b)
{
    uint16_t rgb = 0;

    rgb |= (r >> 3) << 11;
    rgb |= (g >> 2) << 5;
    rgb |= (b >> 3);

    return rgb;
}

void graphics::unpackRGB565(const color_t rgb, uint8_t* r, uint8_t* g, uint8_t* b)
{
    // *r = (rgb >> 11) & 0x1F;
    // *r = (*r * 255) / 31; // Scale back to 8 bits
    //
    // *g = (rgb >> 5) & 0x1F;
    // *g = (*g * 255) / 63; // Scale back to 8 bits
    //
    // *b = rgb & 0x1F;
    // *b = (*b * 255) / 31; // Scale back to 8 bits

    // https://forum.arduino.cc/t/help-converting-rgb565-to-rgb888/275681
    *r = ((((rgb >> 11) & 0x1F) * 527) + 23) >> 6;
    *g = ((((rgb >> 5) & 0x3F) * 259) + 33) >> 6;
    *b = (((rgb & 0x1F) * 527) + 23) >> 6;
}
