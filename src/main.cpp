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

    // Init draw surface
    surface.clear();
    surface.setColor(255, 0, 0);



    while (graphics::isRunning())
    {


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
