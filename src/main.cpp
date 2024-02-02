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
    graphics::init();

    auto canvas = graphics::Surface(graphics::getScreenWidth(), graphics::getScreenHeight());

    while (graphics::isRunning())
    {
        canvas.clear(0, 0, 0);

        canvas.setColor(255, 255, 255);

        canvas.setFontSize(graphics::PT_8);
        canvas.drawText("Hello World !", 16, 16);

        canvas.setFontSize(graphics::PT_12);
        canvas.drawText("Hello World !", 16, 32);

        canvas.setFontSize(graphics::PT_16);
        canvas.drawText("Hello World !", 16, 48);

        canvas.setFontSize(graphics::PT_24);
        canvas.drawText("Hello World !", 16, 64);

        graphics::renderSurface(&canvas);

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
