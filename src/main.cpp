#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#endif

#include "graphics.hpp"
#include "Surface.hpp"
#include "gui.hpp"

// ESP-IDF main
extern "C" void app_main()
{
    graphics::init();

    // auto surface = graphics::Surface(graphics::getScreenWidth(), graphics::getScreenHeight());
    auto surface = graphics::Surface(320, 240);

    uint16_t i = 0;

    while (graphics::isRunning())
    {
        i++;

        surface.clear();
        surface.setColor(255, 0, 0);
        surface.fillRect(i % (graphics::getScreenWidth() - 64), i % (graphics::getScreenHeight() - 64), 64, 64);

        graphics::renderSurface(&surface);
        graphics::flip();

#ifdef ESP_PLATFORM

        vTaskDelay(pdMS_TO_TICKS(200));

#else

        SDL_Delay(200);

#endif
    }
}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char **argv)
{
    graphics::SDLInit(app_main);
}

#endif
