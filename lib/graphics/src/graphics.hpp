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
    enum EScreenOrientation
    {
        PORTRAIT,
        LANDSCAPE
    };

    class Surface;

    void init();
    void reInit();

    uint16_t getScreenWidth();
    uint16_t getScreenHeight();

    void setBrightness(uint16_t value);

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

    void showSurface(const Surface *surface, int x = 0, int y = 0);

    void flip();

    void getTouchPos(int16_t *x, int16_t *y);
    void touchUpdate();
    bool isTouched();

    EScreenOrientation getScreenOrientation();
    void setScreenOrientation(EScreenOrientation screenOrientation);

#ifndef ESP_PLATFORM
    void SDLInit(void (*appMain)());
#endif
}

#endif //GRAPHICS_HPP
