//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "color.hpp"

uint16_t graphics::packRGB565(const uint8_t r, const uint8_t g, const uint8_t b)
{
    uint16_t rgb = 0;

    rgb |= (r >> 3) << 11;
    rgb |= (g >> 2) << 5;
    rgb |= (b >> 3);

    return rgb;
}

void graphics::unpackRGB565(const uint16_t rgb, uint8_t* r, uint8_t* g, uint8_t* b)
{
    *r = (rgb >> 11) & 0x1F;
    *r = (*r * 255) / 31; // Scale back to 8 bits

    *g = (rgb >> 5) & 0x1F;
    *g = (*g * 255) / 63; // Scale back to 8 bits

    *b = rgb & 0x1F;
    *b = (*b * 255) / 31; // Scale back to 8 bits
}
