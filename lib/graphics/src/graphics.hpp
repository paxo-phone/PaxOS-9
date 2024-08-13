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

    extern int16_t brightness;
    void setBrightness(uint16_t value);

    bool isRunning();

    void showSurface(const Surface *surface, int x = 0, int y = 0);
    void setWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
    void setWindow();

    void flip();

    void getTouchPos(int16_t *x, int16_t *y);
    void touchIsRead();
    void touchUpdate();
    bool isTouched();

    EScreenOrientation getScreenOrientation();
    void setScreenOrientation(EScreenOrientation screenOrientation);

#ifndef ESP_PLATFORM
    void SDLInit(void (*appMain)());
#endif
}

#endif //GRAPHICS_HPP
