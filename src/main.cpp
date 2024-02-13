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
    Label* label = new Label(100, 100, 200, 200);
    label->setBackgroundColor(COLOR_ERROR);
    label->setTextColor(COLOR_SUCCESS);
    label->setHorizontalAlignment(Label::Alignement::CENTER);
    label->setVerticalAlignment(Label::Alignement::DOWN);
    label->setText("coucou c'est un super truc intéréssant de voir tout ça hehe nanana");

    win.addChild(label);

    while (graphics::isRunning())
    {
        win.updateAll();
        //box->setX(box->getX() + 20);
        if(label->isTouched())
        {
            label->setX(label->getX() + 20);
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
