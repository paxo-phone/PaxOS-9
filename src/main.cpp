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

using namespace gui::elements;

// ESP-IDF main
extern "C" void app_main()
{
    graphics::init();

    Window win;
    Box* box = new Box(100, 100, 100, 100);
    box->setBackgroundColor(0x1234);

    win.addChild(box);

    while (graphics::isRunning())
    {
        win.updateAll();

#ifdef ESP_PLATFORM

        vTaskDelay(pdMS_TO_TICKS(1000));

#else

        SDL_Delay(10000);

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
