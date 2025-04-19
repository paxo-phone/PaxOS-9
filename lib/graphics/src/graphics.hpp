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
    class Surface;

    enum EScreenOrientation
    {
        PORTRAIT,
        LANDSCAPE
    };

    enum GraphicsInitCode {
        SUCCESS,
        ERROR_NO_TOUCHSCREEN,
        ERROR_FAULTY_TOUCHSCREEN
    };

    /**
     * Initialize the graphics library and returns 0 if initialized with success.
     * @return The initialization code, if greater than 0, an error occurred.
     */
    GraphicsInitCode init();

    [[nodiscard]] uint16_t getScreenWidth();
    [[nodiscard]] uint16_t getScreenHeight();

    /**
     * Get the current brightness of the screen.
     * @return The current brightness of the screen.
     */
    [[nodiscard]] uint16_t getBrightness();

    /**
     * Set the current screen brightness with a smooth animation (abs(newValue - oldValue) ms).
     * @param value The new brightness of the screen.
     * @param temp If true, the brightness value will not be saved (getBrightness() will still return the old value).
     */
    void setBrightness(uint16_t value, bool temp = false);

    /**
     * @return The current running status.
     * @deprecated Please do not use graphics as your main logic.
     */
    [[deprecated("Please do not use graphics as your main logic.")]]
    [[nodiscard]]
    bool isRunning();

    /**
     * Show a surface on the screen.
     * You should probably send every surface to the screen using directly this call,
     * but with a smart use of Surface::pushSurface.
     * @param surface The surface to push to the screen.
     * @param x The x position of the surface, default is 0.
     * @param y The y position of the surface, default is 0.
     */
    void showSurface(const Surface *surface, int x = 0, int y = 0);

    /**
     * Set a clip rect for the screen, only the provided zone will be sent to the LCD.
     * @param x The x of the clip rect.
     * @param y The y of the clip rect.
     * @param width The width of the clip rect.
     * @param height The height of the clip rect.
     */
    void setWindow(uint16_t x, uint16_t y, uint16_t width, uint16_t height);

    /**
     * Reset the previously defined clip rect.
     */
    void setWindow();

    /**
     * Do nothing.
     * Previously used to manually request a screen update.
     * @deprecated Useless.
     */
    [[deprecated("Useless.")]]
    void flip();

    void getTouchPos(int16_t *x, int16_t *y);
    void touchIsRead();
    void touchUpdate();
    [[nodiscard]] bool isTouched();

    [[nodiscard]] EScreenOrientation getScreenOrientation();
    void setScreenOrientation(EScreenOrientation screenOrientation);

    /**
     * Get direct access to the LGFX layer. Please use with a lot of care.
     * @return The LGFX screen instance.
     */
    [[nodiscard]] LGFX* getLCD();
#ifdef ESP_PLATFORM
    [[nodiscard]] FT6236G* getTouchController();
#endif

#ifndef ESP_PLATFORM
    void SDLInit(void (*appMain)());
#endif
}

#endif //GRAPHICS_HPP
