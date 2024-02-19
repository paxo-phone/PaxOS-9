//
// Created by Charlito33 on 23/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "graphics.hpp"

#include <Surface.hpp>

#ifdef ESP_PLATFORM

#include "platforms/LGFX_ESP32_PAXO5.hpp"

#else

#include "lgfx/v1/platforms/sdl/Panel_sdl.hpp"
#include "LGFX_AUTODETECT.hpp"

#include "LovyanGFX.hpp"

#endif

namespace
{
    bool running;

    std::shared_ptr<LGFX> lcd;

    graphics::EScreenOrientation screenOrientation;
    std::shared_ptr<graphics::Surface> landscapeBuffer;
}

void graphics::init()
{
#ifdef ESP_PLATFORM

    running = true; // It doesn't feel right to set this here...

    lcd = std::make_shared<LGFX>();

#else

    lcd = std::make_shared<LGFX>(getScreenWidth(), getScreenHeight());

    // We need to create a "landscape buffer" used as a screen.
    // Because LovyanGFX as a weird color glitch when using "setRotation()".
    // But, by using a temporary buffer, the glitch doesn't appear.
    // TODO : Free memory
    landscapeBuffer = std::make_shared<Surface>(getScreenWidth(), getScreenHeight());

#endif

    lcd->init();
    lcd->setBrightness(0xFF);
    lcd->setColorDepth(16);
    lcd->setTextColor(TFT_WHITE);
    lcd->fillScreen(TFT_BLACK);

    lcd->startWrite(); // Keep the SPI Bus busy ?

#ifdef ESP_PLATFORM
    // uint16_t calibrationData[8];
    // lcd->calibrateTouch(calibrationData, TFT_MAGENTA, TFT_BLACK);

    // Please do a real calibration thing... (see above)
    uint16_t calibrationData[] = {
        390,
        170,
        350,
        3960,
        3800,
        150,
        3900,
        3950
    };

    lcd->setTouchCalibrate(calibrationData);
#endif
}

uint16_t graphics::getScreenWidth()
{
    switch (screenOrientation)
    {
    case graphics::PORTRAIT:
        return 320;
    case graphics::LANDSCAPE:
        return 480;
    }

    return -1;
}

uint16_t graphics::getScreenHeight()
{
    switch (screenOrientation)
    {
    case graphics::PORTRAIT:
        return 480;
    case graphics::LANDSCAPE:
        return 320;
    }

    return -1;
}

bool graphics::isRunning()
{
    return running;
}

#ifndef ESP_PLATFORM

struct SDLUpdateData
{
    void (*appMain)();
};

static int SDLUpdate(void *data)
{
    const auto *updateData = static_cast<SDLUpdateData *>(data);

    updateData->appMain();

    return 0;
}

void graphics::SDLInit(void (*appMain)())
{
    lgfx::Panel_sdl::setup();
    // lgfx::Panel_sdl::loop(); // Ensure to create the window before creating a new thread

    SDLUpdateData updateData
    {
        appMain
    };

    running = true;

    // Multithreading can be an issue, be careful
    SDL_Thread *thread = SDL_CreateThread(SDLUpdate, "graphics_update", &updateData);
    if (thread == nullptr)
    {
        printf("Unable to create thread : %s\n", SDL_GetError());
        exit(1);
    }

    while (lgfx::Panel_sdl::loop() == 0)
    {};

    running = false;

    SDL_WaitThread(thread, nullptr);

    lgfx::Panel_sdl::close();
}

#endif

// You should only use this function with a "Canvas" (Surface that is the size of the screen)
void graphics::showSurface(const Surface* surface, int x, int y)
{
    lgfx::LGFX_Sprite sprite = surface->m_sprite; // we are friends !

#ifdef ESP_PLATFORM
    sprite.pushSprite(lcd.get(), x, y);
#else
    if (screenOrientation == LANDSCAPE)
    {
        landscapeBuffer->pushSurface(const_cast<Surface *>(surface), static_cast<int16_t>(x), static_cast<int16_t>(y));
        landscapeBuffer->m_sprite.pushSprite(lcd.get(), 0, 0);
    }
    else
    {
        sprite.pushSprite(lcd.get(), x, y);
    }
#endif
}

void graphics::flip()
{
    // lcd->display();
}

void graphics::getTouchPos(int16_t* x, int16_t* y)
{
    int16_t tx;
    int16_t ty;

    lcd->getTouch(&tx, &ty);

    if (tx < 0 || ty < 0 || tx > graphics::getScreenWidth() || ty > graphics::getScreenHeight())
    {
        // Be sure to be offscreen
        *x = -1;
        *y = -1;
    }
    else
    {
        *x = tx;
        *y = ty;
    }
}

bool graphics::isTouched()
{
    int16_t x;
    int16_t y;

    getTouchPos(&x, &y);

    return x != -1 && y != -1;
}

graphics::EScreenOrientation graphics::getScreenOrientation()
{
    return screenOrientation;
}

void graphics::setScreenOrientation(const EScreenOrientation screenOrientation)
{
    // Update the screen orientation (and the screen size)
    // Maybe use another name for the parameter ?
    // Or store it in another place ?
    ::screenOrientation = screenOrientation;

    switch (screenOrientation)
    {
    case PORTRAIT:
        lcd->setRotation(0);
        break;
    case LANDSCAPE:
        lcd->setRotation(1);
        break;
    }
}
