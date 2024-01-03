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

    constexpr uint16_t getScreenWidth()
    {
        return 320;
    }

    constexpr uint16_t getScreenHeight()
    {
        return 480;
    }

    void init();

    constexpr int getRenderScale()
    {
        // SDL2 does not support scaling
#ifdef ESP_PLATFORM
        return 2; // Change this value
#else
        return 1; // Don't change this value
#endif
    }

    bool isRunning();

    void renderSurface(const Surface *surface);

    void flip();

    void getTouchPos(int16_t *x, int16_t *y);

#ifndef ESP_PLATFORM
    void SDLInit(void (*appMain)());
#endif
}

#endif //GRAPHICS_HPP
