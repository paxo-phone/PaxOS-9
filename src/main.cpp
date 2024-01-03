#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_pm.h"

#endif

#include "graphics.hpp"
#include "Surface.hpp"
#include "gui.hpp"

#include "delay.hpp"

// ESP-IDF main
extern "C" void app_main()
{
    graphics::init();

    auto surface = graphics::Surface(graphics::getScreenWidth(), graphics::getScreenHeight());
    // auto surface2 = graphics::Surface(100, 100);

    // Init draw surface
    surface.clear();
    surface.setColor(255, 0, 0);

    uint16_t i = 0;

    int16_t touchX = -1;
    int16_t touchY = -1;

    int16_t oldTouchX = -1;
    int16_t oldTouchY = -1;

    while (graphics::isRunning())
    {
        i++;

        // If screen is touched
        if (graphics::isTouched())
        {
            // Default case remove
            if (touchX != -1 && touchY != -1)
            {
                oldTouchX = touchX;
                oldTouchY = touchY;
            }

            graphics::getTouchPos(&touchX, &touchY);

            printf("Touch : %d %d\n", touchX, touchY);

            // Default case remove
            if (oldTouchX != -1 && oldTouchY != -1)
            {
                // Draw a line between the touch points
                surface.drawLine(oldTouchX, oldTouchY, touchX, touchY);
            }
        }
        else
        {
            // Reset values ?
            touchX = -1;
            touchY = -1;
            oldTouchX = -1;
            oldTouchY = -1;
        }

        // surface2.clear();
        // surface2.setColor(0, 255, 0);
        // surface2.fillRect(static_cast<int16_t>(i % (100 - 20)), static_cast<int16_t>(i % (100 - 20)), 20, 20);

        // surface.clear(0, 0, 255);
        // surface.pushSurface(&surface2, i % (graphics::getScreenWidth() - 100), i % (graphics::getScreenHeight() - 100));

        // surface.pushSurface(&surface2, static_cast<int16_t>(touchX - 50), static_cast<int16_t>(touchY - 50));

        graphics::renderSurface(&surface);
        graphics::flip();

        temp::delay(1); // Don't trigger the watchdog
    }
}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char **argv)
{
    graphics::SDLInit(app_main);
}

#endif
