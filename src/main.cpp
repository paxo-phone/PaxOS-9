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

    auto image = graphics::Image(graphics::BMP);
    image.loadBMP("resources/images/logo.bmp");

    canvas.drawImage(image, 0, 0);

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
