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

        canvas.setFontSize(8);
        canvas.drawText("Hello World !", 16, 16);

        canvas.setFontSize(12);
        canvas.drawText("Hello World !", 16, 64);

        canvas.setFontSize(14);
        canvas.drawText("Hello World !", 16, 96);

        canvas.setFontSize(16);
        canvas.drawText("Hello World !", 16, 128);

        canvas.setFontSize(18);
        canvas.drawText("Hello World !", 16, 164);

        canvas.setFontSize(20);
        canvas.drawText("Hello World !", 16, 192);

        canvas.setFontSize(24);
        canvas.drawText("Hello World !", 16, 256);

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
