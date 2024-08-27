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
#include "GuiManager.hpp"
#include "unistd.h"


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
    graphics::setBrightness(graphics::brightness);
    #endif
    
    GuiManager &guiManager = GuiManager::getInstance();

    // Main loop
    while (true) {
        int l = -1;

        // if there is no app running, run launcher, and if an app is choosen, launch it
        if(!AppManager::isAnyVisibleApp() && (l = launcher()) != -1)    
        {
            int search = 0;

            for (int i = 0; i < AppManager::appList.size(); i++)
            {
                if(AppManager::appList[i].visible)
                {
                    if(search == l)
                    {
                        try{
                            AppManager::get(i).run(false);
                        }
                        catch(std::runtime_error &e) { // Si Erreur à l'execution de l'app, on catche et affiche un msg d'ereur
                            std::cerr << "Erreur: " << e.what() << std::endl;
                            // Affichage du msg d'erreur
                            guiManager.showErrorMessage(e.what());
                            // on kill l'application ?!?
                            //AppManager::appList[i].kill();
                        }

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
            graphics::setBrightness(graphics::brightness);
        }

        AppManager::loop();
    }
}

void setup()
{
    /**
     * Initialisation du hardware, de l'écran, lecture des applications stcokées dans storage
     */
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

    // Positionnement de l'écran en mode Portrait
    graphics::setScreenOrientation(graphics::PORTRAIT);

    // Init de la gestiuon des Threads
    ThreadManager::init();

    /**
     * Gestion des eventHandlers pour les evenements
     */

    // gestion des appels entrants
    GSM::ExternalEvents::onIncommingCall = []()
    {
        eventHandlerApp.setTimeout(new Callback<>([](){AppManager::get(".receivecall").run(false);}), 0);
    };

    // Gestion de la réception d'un message
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

    // gestion de la détection du toucher de l'écran
    eventHandlerBack.setInterval(
        &graphics::touchUpdate,
        10
    );

    hardware::setVibrator(false);
    GSM::endCall();

    // Chargement des contacts
    std::cout << "[Main] Loading Contacts" << std::endl;
    Contacts::load();

    std::vector<Contacts::contact> cc = Contacts::listContacts();
    
/*    for(auto c : cc)
    {
        //std::cout << c.name << " " << c.phone << std::endl;
    }
*/

    /**
     * Gestion des applications
     */
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
