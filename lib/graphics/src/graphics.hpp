//
// Created by Charlito33 on 23/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#define LGFX_USE_V1

#include <cstdint>

#ifdef ESP_PLATFORM

#include "platforms/LGFX_ESP32_PAXO5.hpp"

#else

#include "lgfx/v1/platforms/sdl/Panel_sdl.hpp"
#include "LGFX_AUTODETECT.hpp"

#endif

class FT6236G;

namespace graphics
{
    enum EScreenOrientation
    {
        PORTRAIT,
        LANDSCAPE
    };

    class Surface;

    LGFX* getLcd();
    void reInit();

  enum GraphicsInitCode {
        SUCCESS,
        ERROR_NO_TOUCHSCREEN,
        ERROR_FAULTY_TOUCHSCREEN
    };

    GraphicsInitCode init();

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

    LGFX* getLGFX();
#ifdef ESP_PLATFORM
    FT6236G* getTouchController();
#endif

#ifndef ESP_PLATFORM
    void SDLInit(void (*appMain)());
#endif
}

#endif //GRAPHICS_HPP
