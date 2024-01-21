#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_pm.h"

#endif

#include "graphics.hpp"
#include "Surface.hpp"
#include "gui.hpp"

#include "delay.hpp"
#include "elements/Box.hpp"
#include "arial.h"
#include <iostream>

// ESP-IDF main
extern "C" void app_main()
{
    graphics::init();

    auto screen = graphics::Surface(graphics::getScreenWidth(), graphics::getScreenHeight());
    screen.m_sprite.setFont(&Arial12ptLatin1);
    gui::setRenderTarget(screen);
    std::string text = "aze";
    text+=0xa0;
    std::string text2 = "é";
    std::cout << "char: " << int(text2.size()) << std::endl;
    screen.m_sprite.print(text.c_str());

    auto box = gui::elements::Box(16, 16, 64, 64);

    box.setColor(58, 186, 153);

    while (graphics::isRunning())
    {
        //box.renderAll();

        // Flip screen
        graphics::renderSurface(&screen);
        graphics::flip();

        temp::delay(1); // Watchdog
    }
}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char **argv)
{
    graphics::SDLInit(app_main);
}

#endif
