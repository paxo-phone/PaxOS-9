#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#endif

#include "graphics.hpp"
#include "hardware.hpp"
#include "gui.hpp"
#include "path.hpp"
#include "filestream.hpp"
#include <iostream>

using namespace gui::elements;

void loop(){}

void setup()
{
    hardware::init();
    hardware::setScreenPower(true);
    
    graphics::init();

    Window win;
    Input* in = new Input(35, 35, 0, 0);
    in->setTitle("Prénom:");
    in->setPlaceHolder("écrire ici");

    win.addChild(in);

    /*Switch* sw1 = new Switch(10, 100);
    Switch* sw2 = new Switch(10, 150);
    sw2->setState(true);*/

    VerticalList* list = new VerticalList(40, 100, 40, 300);
    for (uint16_t i = 0; i < 3; i++)
        list->add(new Switch(0, 0));
    list->add(new Checkbox(0, 0));
    list->add(new Radio(0, 0));


    win.addChild(list);

    Button* button = new Button(35, 394, 250, 38);
    button->setText("bonjour");
    button->setIcon(storage::Path("icon.png"));

    //win.addChild(label);

    win.addChild(button);

    while (graphics::isRunning())
    {
        win.updateAll();

        if(in->isTouched())
        {
            //in->setX(in->getX() + 20);
            in->setText("C'est Gabriel");
        }

        /*if(sw1->isTouched())
        {
            if(sw1->getState())
                in->enable();
            else
                in->disable();
        }*/

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
    graphics::SDLInit(setup);
}

#endif
