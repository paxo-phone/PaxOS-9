#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <esp_system.h>

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

    GSM::ExternalEvents::onIncommingCall = []()
    {
        app::App call;
        call.auth = true;
        call.name = "phone";
        call.manifest = storage::Path("/sys_apps/root.json");
        call.path = storage::Path("/sys_apps/call.lua");

        app::requestingApp = call;
        app::request = true;
    };

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

    while (true)
    {
        #ifdef ESP_PLATFORM
        for (uint16_t i = 0; i < 0xFF/3; i++)
        {
            graphics::setBrightness(i);
            delay(2);
        }
        #endif

        int l = 0;
        while (l!=-1)
        {
            l = launcher();
            if(l!=-1)
                app::runApp(app::appList[l].path);

            while (hardware::getHomeButton());
        }

        #ifdef ESP_PLATFORM
        for (uint16_t i = 0xFF/3; i > 0; i--)
        {
            graphics::setBrightness(i);
            delay(2);
        }
        
        graphics::setBrightness(0);
        setCpuFrequencyMhz(20);
        GSM::reInit();
        //hardware::setScreenPower(false);
        #endif

        while (hardware::getHomeButton());
        while (!hardware::getHomeButton())
        {
            eventHandlerApp.update();
        }
        
        #ifdef ESP_PLATFORM
        setCpuFrequencyMhz(240);
        GSM::reInit();
        #endif
    }

    while (graphics::isRunning())
    {
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
    std::cout << "WTF BRO" << std::endl;
    graphics::SDLInit(setup);
}

#endif
