//
// Created by Charlito33 on 23/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "graphics.hpp"

#include <iostream>
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
}

void graphics::init()
{
#ifdef ESP_PLATFORM

    running = true; // It doesn't feel right to set this here...

    lcd = std::make_shared<LGFX>();

#else

    lcd = std::make_shared<LGFX>(getScreenWidth(), getScreenHeight());

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
    if (x != 0 || y != 0)
    {
        std::cerr << "---------------------------------------------------------------------------------------------------------------------" << std::endl;
        std::cerr << "                                                 Warning !                                                           " << std::endl;
        std::cerr << "                                                                                                                     " << std::endl;
        std::cerr << ">>> You are using 'graphics::showSurface(...)' with a non-zero 'x' and/or 'y' value.                              <<<" << std::endl;
        std::cerr << ">>> This is deprecated and will be removed.                                                                       <<<" << std::endl;
        std::cerr << ">>> Please push to a 'graphics::Surface' before pushing to the screen.                                            <<<" << std::endl;
        std::cerr << ">>> By using a 'graphics::Surface' before pusing to the screen, you are also enabling double buffering rendering. <<<" << std::endl;
        std::cerr << "---------------------------------------------------------------------------------------------------------------------" << std::endl;
    }

    lgfx::LGFX_Sprite sprite = surface->m_sprite; // we are friends !

    sprite.pushSprite(lcd.get(), x, y);
}

void graphics::flip()
{
    lcd->display();
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