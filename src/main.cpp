#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_pm.h"

#endif

#include "graphics.hpp"

#include "Surface.hpp"
#include "Image.hpp"

#include "delay.hpp"

// ESP-IDF main
extern "C" void app_main()
{
    graphics::init();

    auto canvas = graphics::Surface(graphics::getScreenWidth(), graphics::getScreenHeight());

    canvas.setColor(60, 60, 60);
    canvas.fillRect(0, 0, 320, 20);

    canvas.setColor(255, 255, 255);
    canvas.setTextScale(2);
    canvas.drawText("Hello World !", 0, 2);

    canvas.setColor(60, 60, 60);
    canvas.fillRoundRect(20, 400, 280, 60, 30);

    while (graphics::isRunning())
    {
        graphics::renderSurface(&canvas);
        graphics::flip();

        temp::delay(1);
    }
}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char **argv)
{
    graphics::SDLInit(app_main);

    return 0;
}

#endif
