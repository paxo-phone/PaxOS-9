//
// Created by Charlito33 on 23/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "graphics.hpp"

#ifdef ESP_PLATFORM

#include "platforms/LGFX_ESP32.hpp"

#else

#include "lgfx/v1/platforms/sdl/Panel_sdl.hpp"
#include "LGFX_AUTODETECT.hpp"

#endif

namespace
{
    bool running;

    std::shared_ptr<LGFX> lcd;
}

void graphics::init()
{
#ifdef ESP_PLATFORM

    lcd = std::make_shared<LGFX>();

#else

    lcd = std::make_shared<LGFX>(screenWidth, screenHeight);

#endif

    lcd->init();
    lcd->setBrightness(0xFF);
    lcd->setColorDepth(16);
    lcd->setTextColor(TFT_WHITE);
    lcd->fillScreen(TFT_BLACK);
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

    SDLUpdateData updateData {
        appMain
    };

    running = true;

    // Multithreading can be an issue, be careful
    SDL_Thread *thread = SDL_CreateThread(SDLUpdate, "graphics_update", &updateData);
    if (thread == nullptr) {
        printf("Unable to create thread : %s\n", SDL_GetError());
        exit(1);
    }

    while (lgfx::Panel_sdl::loop() == 0) {};

    running = false;

    SDL_WaitThread(thread, nullptr);

    lgfx::Panel_sdl::close();
}

#endif

void graphics::setColor(const uint8_t r, const uint8_t g, const uint8_t b)
{
    lcd->setColor(r, g, b);
}

void graphics::fillRect(const uint16_t x, const uint16_t y, const uint16_t w, const uint16_t h)
{
    lcd->fillRect(x, y, w, h);
}
