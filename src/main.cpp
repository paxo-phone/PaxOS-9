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
    graphics::setScreenOrientation(graphics::LANDSCAPE);

    Window win;
    Input* in = new Input(35, 35, 0, 0);
    in->setTitle("Prénom:");
    in->setPlaceHolder("palce holder");

    win.addChild(in);

    while (graphics::isRunning())
    {
        win.updateAll();

        if(in->isTouched())
        {
            //in->setX(in->getX() + 20);
            in->setText("C'est Gabriel");
        }

#ifdef ESP_PLATFORM

        vTaskDelay(pdMS_TO_TICKS(10));

#else

        SDL_Delay(10);

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
