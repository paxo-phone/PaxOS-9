#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#endif

#include "graphics.hpp"
#include "Surface.hpp"

// ESP-IDF main
extern "C" void app_main()
{
    graphics::init();

    auto canvas = graphics::Surface(graphics::getScreenWidth(), graphics::getScreenHeight());

    auto box = graphics::Surface(64, 64);
    box.clear(255, 0, 0);

    canvas.pushSurface(&box, 0, 0);

    graphics::renderSurface(&canvas);

    while (graphics::isRunning())
    {


#ifdef ESP_PLATFORM

        vTaskDelay(pdMS_TO_TICKS(1000));

#else

        SDL_Delay(1000);

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
