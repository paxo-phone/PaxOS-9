//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#ifndef COLOR_HPP
#define COLOR_HPP

#include <cstdint>

namespace graphics
{
    uint16_t packRGB565(uint8_t r, uint8_t g, uint8_t b);
    void unpackRGB565(uint16_t rgb, uint8_t *r, uint8_t *g, uint8_t *b);
}

#endif //COLOR_HPP
