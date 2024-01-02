//
// Created by Charlito33 on 23/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <cstdint>

namespace graphics
{
    class Surface;

    constexpr uint16_t screenWidth = 320;
    constexpr uint16_t screenHeight = 480;

    void init();
    void close();

    bool isRunning();

#ifndef ESP_PLATFORM
    void SDLInit(void (*appMain)());
#endif

    void setColor(uint8_t r, uint8_t g, uint8_t b);

    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
}

#endif //GRAPHICS_HPP
