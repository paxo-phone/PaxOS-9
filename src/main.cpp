#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <esp_system.h>
#include <backtrace_saver.hpp>
#include "backtrace.hpp"
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
#include "contacts.hpp"
#include <iostream>

using namespace gui::elements;



void ringingVibrator(void* data)
{
    #ifdef ESP_PLATFORM
    while (true)
    {
        if(GSM::state.callState == GSM::CallState::RINGING)
        {
            delay(200); hardware::setVibrator(true); delay(100); hardware::setVibrator(false);
        }
        delay(10);
    }
    #endif
}

void mainLoop(void* data)
{
    #ifdef ESP_PLATFORM
    graphics::setBrightness(0xFF/3);
    #endif
    
    while (true)    // Main loop
    {
        int l = -1;

        if(!AppManager::isAnyVisibleApp() && (l = launcher()) != -1)    // if there is not app running, run launcher, and it an app is choosen, launch it
        {
            int search = 0;

            for (int i = 0; i < AppManager::appList.size(); i++)
            {
                if(AppManager::appList[i].visible)
                {
                    if(search == l)
                    {
                        AppManager::get(i).run(false);
                        while (AppManager::isAnyVisibleApp())
                            AppManager::loop();
                        break;
                    }
                    search++;
                }
            }
        }

        if(!AppManager::isAnyVisibleApp() && l == -1)   // if the launcher did not launch an app and there is no app running, then sleep
        {
            graphics::setBrightness(0);
            StandbyMode::savePower();

            while (hardware::getHomeButton());
            while (!hardware::getHomeButton() && !AppManager::isAnyVisibleApp()/* && GSM::state.callState != GSM::CallState::RINGING*/)
            {
                eventHandlerApp.update();
                AppManager::loop();
            }

            while (hardware::getHomeButton());
            
            StandbyMode::restorePower();
            graphics::setBrightness(0xFF/3);
        }

        AppManager::loop();
    }
}

void setup()
{
    hardware::init();
    hardware::setScreenPower(true);
    graphics::init();
    storage::init();
    #ifdef ESP_PLATFORM
    backtrace_saver::init();

    backtrace_saver::backtraceEventId = eventHandlerBack.addEventListener(
        new Condition<>(&backtrace_saver::shouldSaveBacktrace),
        new Callback<>(&backtrace_saver::saveBacktrace)
    );
    #endif // ESP_PLATFORM

    graphics::setScreenOrientation(graphics::PORTRAIT);

    ThreadManager::init();

    GSM::ExternalEvents::onIncommingCall = []()
    {
        eventHandlerApp.setTimeout(new Callback<>([](){AppManager::get(".receivecall").run(false);}), 0);
    };

    GSM::ExternalEvents::onNewMessage = []()
    {
        #ifdef ESP_PLATFORM
        eventHandlerBack.setTimeout(new Callback<>([](){delay(200); hardware::setVibrator(true); delay(100); hardware::setVibrator(false);}), 0);
        #endif
        
        AppManager::event_onmessage();
    };

    GSM::ExternalEvents::onNewMessageError = []()
    {
        AppManager::event_onmessageerror();
    };

    #ifdef ESP_PLATFORM
    ThreadManager::new_thread(CORE_BACK, &ringingVibrator, 16000);
    #endif

    eventHandlerBack.setInterval(
        &graphics::touchUpdate,
        10
    );

    hardware::setVibrator(false);
    GSM::endCall();

    std::cout << "[Main] Loading Contacts" << std::endl;
    Contacts::load();

    std::vector<Contacts::contact> cc = Contacts::listContacts();
    
    for(auto c : cc)
    {
        //std::cout << c.name << " " << c.phone << std::endl;
    }

    app::init();
    AppManager::init();

    #ifdef ESP_PLATFORM
    xTaskCreateUniversal(mainLoop,"newloop", 32*1024, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
    vTaskDelete(NULL);
    #else
    mainLoop(NULL);
    #endif
}

void loop(){}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char **argv)
{
    graphics::SDLInit(setup);
}

#endif
