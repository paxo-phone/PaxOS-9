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
#include "elements/Box.hpp"

// ESP-IDF main
extern "C" void app_main()
{
    graphics::init();

    auto screen = graphics::Surface(graphics::getScreenWidth(), graphics::getScreenHeight());

    gui::setRenderTarget(screen);

    auto box = gui::elements::Box(16, 16, 64, 64);

    while (graphics::isRunning())
    {
        box.renderAll();

        // Flip screen
        graphics::renderSurface(&screen);
        graphics::flip();

        temp::delay(1); // Watchdog
    }
}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char **argv)
{
    graphics::SDLInit(app_main);
}

#endif
