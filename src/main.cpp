#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#endif

#include "graphics.hpp"

// ESP-IDF main
extern "C" void app_main()
{
    graphics::init();
    graphics::setColor(255, 0, 0);
    graphics::fillRect(0, 0, 64, 64);

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
