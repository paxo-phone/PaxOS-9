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

    canvas.clear(0, 0, 0);

    // Draw wallpaper
    const auto wallpaper = graphics::Image("resources/images/wallpaper.png");
    canvas.drawImage(wallpaper, 0, 0);

    // Draw status bar
    canvas.setColor(40, 40, 40);
    canvas.fillRect(0, 0, 320, 20);

    // Draw time
    canvas.setColor(255, 255, 255);
    canvas.setTextScale(2);
    canvas.drawTextCentered("13:38", 0, 3, 320);

    // Draw apps
    canvas.setColor(0, 0, 0);
    int16_t appX;
    int16_t appY = 36;
    for (uint8_t i = 0; i < 4; i++)
    {
        appX = 8;

        for (uint8_t j = 0; j < 4; j++)
        {
            canvas.fillRoundRect(appX, appY, 64, 64, 16);

            appX += 80;
        }

        appY += 80;
    }

    // Draw dock
    canvas.setColor(60, 60, 60);
    canvas.fillRoundRect(8, 396, 304, 76, 20);

    // Draw dock - Apps
    canvas.setColor(0, 0, 0);
    appX = 8 + 6;
    for (uint8_t i = 0; i < 4; i++)
    {
        canvas.fillRoundRect(appX, 396 + 6, 64, 64, 16);
        appX += 64 + 12;
    }

    // Markers
    if (false)
    {
        // Draw markers
        canvas.setColor(127, 0, 0);
        canvas.drawLine(10, 0, 10, 480);
        canvas.drawLine(310, 0, 310, 480);
        canvas.drawLine(0, 20, 320, 20);

        // Draw markers - App dock
        canvas.drawLine(0, 400, 320, 400);
        canvas.drawLine(0, 474, 320, 474);

        // Draw markers - Apps
        canvas.setColor(0, 0, 255);

        // Draw markers - Apps - Rows
        int16_t rowX = 8;
        canvas.drawLine(rowX, 0, rowX, 480);
        for (uint8_t i = 0; i < 4; i++)
        {
            rowX += 64;
            canvas.drawLine(rowX, 0, rowX, 480);
            rowX += 16;
            canvas.drawLine(rowX, 0, rowX, 480);
        }

        // Draw markers - Apps - Cols
        int16_t colY = 36;
        canvas.drawLine(0, colY, 320, colY);
        for (uint8_t i = 0; i < 4; i++)
        {
            colY += 64;
            canvas.drawLine(0, colY, 320, colY);
            colY += 16;
            canvas.drawLine(0, colY, 320, colY);
        }

        // Draw preview text
        canvas.setColor(200, 200, 200);
        canvas.setTextScale(1);
        canvas.drawText("PaxOS 9 (lib/graphics test)", 0, 2);
    }

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
