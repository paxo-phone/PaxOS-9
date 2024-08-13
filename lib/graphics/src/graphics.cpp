//
// Created by Charlito33 on 23/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "graphics.hpp"

#include <iostream>
#include <Surface.hpp>
#include <threads.hpp>

#ifdef ESP_PLATFORM

#include "platforms/LGFX_ESP32_PAXO5.hpp"
#include <FT6236G.h>
FT6236G ct;

#else

#include "lgfx/v1/platforms/sdl/Panel_sdl.hpp"
#include "LGFX_AUTODETECT.hpp"

#include "LovyanGFX.hpp"

#endif

int16_t graphics::brightness = 0xFF/3;

namespace
{
    bool running;


    std::shared_ptr<LGFX> lcd;

    graphics::EScreenOrientation screenOrientation;
    std::shared_ptr<graphics::Surface> landscapeBuffer;

    int16_t touchX = 0, touchY = 0;

    bool liveTouchState = false;    // is screen touched (brute and fast)
    int16_t newTouchX = 0, newTouchY = 0;

    bool isTouchRead = false;
}

void graphics::touchIsRead()
{
    isTouchRead = true;
}

void graphics::setBrightness(uint16_t value)
{
    #ifdef ESP_PLATFORM
    static uint16_t oldValue = 0;

    for (uint16_t i = oldValue; i < value; i++)
    {
        lcd->setBrightness(i);
        delay(2);
    }

    for (int16_t i = oldValue; i >= value && i!=-1; i--)
    {
        lcd->setBrightness(i);
        delay(2);
    }

    oldValue = value;
    #else
    if(value == 0)
        lcd->fillScreen(0x0000);
    #endif
}

void graphics::init()
{
#ifdef ESP_PLATFORM

    running = true; // It doesn't feel right to set this here...

    lcd = std::make_shared<LGFX>();
    ct.init(21, 22, false, 400000);

#else

    lcd = std::make_shared<LGFX>(getScreenWidth(), getScreenHeight());

    // We need to create a "landscape buffer" used as a screen.
    // Because LovyanGFX as a weird color glitch when using "setRotation()".
    // But, by using a temporary buffer, the glitch doesn't appear.
    landscapeBuffer = std::make_shared<Surface>(getScreenHeight(), getScreenWidth());

#endif

    lcd->init();
    lcd->setColorDepth(16);
    lcd->setTextColor(TFT_WHITE);
    lcd->fillScreen(TFT_RED);
}

void graphics::reInit()
{
    lcd->init();
    lcd->setTextColor(TFT_WHITE);
    lcd->fillScreen(TFT_RED);

    #ifdef ESP_PLATFORM
    ct.init(21, 22, false, 400000);
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

    // SDL_WaitThread(thread, nullptr);

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

void graphics::setWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    lcd.get()->setWindow(x, y, width, height);
}

void graphics::setWindow()
{
    lcd.get()->setWindow(0, 0, getScreenWidth(), getScreenHeight());
}

void graphics::flip()
{
    // lcd->display();
}

void graphics::getTouchPos(int16_t* x, int16_t* y)
{
    *x = touchX;
    *y = touchY;
}

#ifdef ESP_PLATFORM
int getTouch(uint16_t *pPoints)
{
  TOUCHINFO ti;
  if (ct.getSamples(&ti) != FT_SUCCESS)
     return 0; // something went wrong
    if (pPoints) {
      // swap X/Y since the display is used 90 degrees rotated
      pPoints[0] = ti.x[0];
      pPoints[1] = ti.y[0]; 
      pPoints[2] = ti.x[1];
      pPoints[3] = ti.y[1];
    }
  return ti.count;
}
#endif

void graphics::touchUpdate()
{
    bool touchState = true;
    int16_t liveTouchX = 0, liveTouchY = 0;

    #ifdef ESP_PLATFORM
        uint16_t points[4];
        int i = getTouch(points);
        if(i == 1)
        {
            if(screenOrientation == PORTRAIT)
            {
                liveTouchX = (points[0]-16) * 320 / 303;
                liveTouchY = (points[1]-23) * 480 / 442;
            }
            else
            {
                liveTouchX = (points[1]-23) * 480 / 442;
                liveTouchY = 320 - (points[0]-16) * 320 / 303 -10;
            }
        }
        else
        {
            liveTouchX = -1;
            liveTouchY = -1;
        }
    #else
        bool state = lcd->getTouch(&liveTouchX, &liveTouchY);

        if(!state)
        {
            liveTouchX = -1;
            liveTouchY = -1;
        }
    #endif

    if (liveTouchX <= 0 || liveTouchY <= 0 || liveTouchX > graphics::getScreenWidth() || liveTouchY > graphics::getScreenHeight())
    {
        // Be sure to be offscreen
        liveTouchX = -1;
        liveTouchY = -1;

        touchState = false;
    }

    if(touchState == true) // si il vient d'être touché -> sauvegarder
    {
        newTouchX = liveTouchX;
        newTouchY = liveTouchY;
    }

    if(isTouchRead) // envoyer la nouvelle valeur
    {
        touchX = newTouchX;
        touchY = newTouchY;
        newTouchX = -1;
        newTouchY = -1;
        isTouchRead = false;
    }
}

bool graphics::isTouched()
{
    return touchX != -1 && touchY != -1;
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
