#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#endif

#include "graphics.hpp"
#include "gui.hpp"
#include "path.hpp"
#include "filestream.hpp"
#include <iostream>

// ESP-IDF main
extern "C" void app_main()
{
    storage::init();

    storage::Path path("test.txt");
    storage::FileStream stream(path.str(), storage::Mode::READ);
    std::cout << "content: " << stream.read() << std::endl;

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
