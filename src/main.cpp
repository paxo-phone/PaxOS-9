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
#include "threads.hpp"
#include "lua_file.hpp"
#include "gsm.hpp"
#include "app.hpp"
#include <iostream>

using namespace gui::elements;

void loop(){}

void ringingVibrator()
{
    #ifdef ESP_PLATFORM
    if(GSM::state.callState == GSM::CallState::RINGING)
    {
        hardware::setVibrator(true); delay(100); hardware::setVibrator(false);
    }
    #endif
}

void setup()
{
    hardware::init();
    hardware::setScreenPower(true);
    graphics::init();
    storage::init();

    graphics::setScreenOrientation(graphics::PORTRAIT);

    ThreadManager::init();

    #ifdef ESP_PLATFORM
    eventHandlerBack.setInterval(
        new Callback<>(&ringingVibrator),
        300
    );
    #endif

    app::init();
    for (auto a : app::appList)
    {
        std::cout << a.name << ";" << a.path.str() << std::endl;
    }
    app::runApp(app::appList[0].path);

    /*LuaFile lua(storage::Path("/app.lua"));
    lua.run();

    gui::elements::Window win;
    Input* in = new Input(35, 35);
    in->setTitle("Prénom:");
    in->setPlaceHolder("écrire ici");

    win.addChild(in);

    VerticalList* list = new VerticalList(40, 100, 40, 300);
    for (uint16_t i = 0; i < 3; i++)
        list->add(new Switch(0, 0));
    list->add(new Checkbox(0, 0));
    list->add(new Radio(0, 0));


    win.addChild(list);

    Button* button = new Button(35, 394, 250, 38);
    button->setText("bonjour");
    button->setIcon(storage::Path("/icon.png"));

    //win.addChild(label);

    win.addChild(button);*/

    while (graphics::isRunning())
    {
        /*win.updateAll();
        

        if(in->isTouched())
        {
            in->setText("C'est Gabriel");
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
