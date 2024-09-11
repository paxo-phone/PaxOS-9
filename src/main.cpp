#ifdef ESP_PLATFORM

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include <backtrace_saver.hpp>
#include <backtrace.hpp>

#endif

#include <unistd.h>

#include <graphics.hpp>
#include <hardware.hpp>
#include <gui.hpp>
#include <path.hpp>
#include <threads.hpp>
#include <lua_file.hpp>
#include <gsm.hpp>
#include <app.hpp>
#include <contacts.hpp>
#include <FileConfig.hpp>
#include <iostream>
#include <libsystem.hpp>
#include <GuiManager.hpp>


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

void mainLoop(void* data) {
#ifdef ESP_PLATFORM
    if (!backtrace_saver::isBacktraceEmpty()) {
        backtrace_saver::backtraceMessageGUI();
    }

    // Initialize brightness in some way
    graphics::setBrightness(graphics::getBrightness());
#endif

    GuiManager& guiManager = GuiManager::getInstance();

    // ReSharper disable once CppTooWideScopeInitStatement
    const libsystem::FileConfig systemConfig = libsystem::getSystemConfig();

    // Check if OOBE app need to be launched
    if (!systemConfig.has("oobe") || !systemConfig.get<bool>("oobe")) {
        // Launch OOBE app
        const std::shared_ptr<AppManager::App> oobeApp = AppManager::get(".oobe");

        try {
            oobeApp->run(false);
        } catch (std::runtime_error& e) {
            std::cerr << "Lua error: " << e.what() << std::endl;
            guiManager.showErrorMessage(e.what());
            //AppManager::appList[i].kill();
        }
    }

    // Main loop
    while (true) {
        // Update inputs
        hardware::input::update();

        // Update running apps
        AppManager::update();

        // Don't show anything
        if (libsystem::getDeviceMode() == libsystem::SLEEP) {
            if (getButtonDown(hardware::input::HOME)) {
                setDeviceMode(libsystem::NORMAL);
            }

            continue;
        }

        if (AppManager::isAnyVisibleApp()) {
            if (getButtonDown(hardware::input::HOME)) {
                AppManager::quitApp();
            }
        } else {
            // If home button pressed on the launcher
            // Put the device in sleep
            if (getButtonDown(hardware::input::HOME)) {
                // Free the launcher resources
                applications::launcher::free();

                setDeviceMode(libsystem::SLEEP);
                continue;
            }

            // Update, show and allocate launcher
            applications::launcher::update();

            // Icons interactions
            if (applications::launcher::iconTouched()) {
                const std::shared_ptr<AppManager::App> app = applications::launcher::getApp();

                // Free the launcher resources
                applications::launcher::free();

                // Launch the app
                try {
                    app->run(false);
                } catch (std::runtime_error& e) {
                    std::cerr << "Erreur: " << e.what() << std::endl;
                    // Affichage du msg d'erreur
                    guiManager.showErrorMessage(e.what());
                    // on kill l'application ?!?
                    //AppManager::appList[i].kill();
                }
            }
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

    // Init graphics and check for errors
    if (const graphics::GraphicsInitCode graphicsInitCode = graphics::init(); graphicsInitCode != graphics::SUCCESS) {
        libsystem::registerBootError("Graphics initialization error.");

        if (graphicsInitCode == graphics::ERROR_NO_TOUCHSCREEN) {
            libsystem::registerBootError("No touchscreen found.");
        } else if (graphicsInitCode == graphics::ERROR_FAULTY_TOUCHSCREEN) {
            libsystem::registerBootError("Faulty touchscreen detected.");
        }
    }
    setScreenOrientation(graphics::PORTRAIT);

    // If battery is too low
    // Don't initialize ANY MORE service
    // But display error
    if (GSM::getBatteryLevel() < 0.05 && !hardware::isCharging()) {
        libsystem::registerBootError("Battery level is too low.");
        libsystem::registerBootError(std::to_string(static_cast<int>(GSM::getBatteryLevel() * 100)) + "% < 5%");
        libsystem::registerBootError("Please charge your Paxo.");
        libsystem::registerBootError("Tip: Force boot by plugging a charger.");

        libsystem::displayBootErrors();
        libsystem::restart(true, 10000);

        return;
    }

    // Set device mode to normal
    setDeviceMode(libsystem::NORMAL);

    // Init storage and check for errors
    if (!storage::init()) {
        libsystem::registerBootError("Storage initialization error.");
        libsystem::registerBootError("Please check the SD Card.");
    }

    #ifdef ESP_PLATFORM
    backtrace_saver::init();
    std::cout << "backtrace: " << backtrace_saver::getBacktraceMessage() << std::endl;
    backtrace_saver::backtraceEventId = eventHandlerBack.addEventListener(
        new Condition<>(&backtrace_saver::shouldSaveBacktrace),
        new Callback<>(&backtrace_saver::saveBacktrace)
    );
    #endif // ESP_PLATFORM

    // Init de la gestiuon des Threads
    ThreadManager::init();

    libsystem::FileConfig systemConfig = libsystem::getSystemConfig();

    if (!systemConfig.has("settings.brightness")) {
        systemConfig.set<uint8_t>("settings.brightness", 69);
        systemConfig.write();
    }

    libsystem::log("settings.brightness: " + std::to_string(systemConfig.get<uint8_t>("settings.brightness")));

    graphics::setBrightness(systemConfig.get<uint8_t>("settings.brightness"));

    // Init launcher
    applications::launcher::init();

    // When everything is initialized
    // Check if errors occurred
    // If so, restart
    if (libsystem::hasBootErrors()) {
        libsystem::displayBootErrors();
        libsystem::restart(true, 10000);
    }

    /**
     * Gestion des eventHandlers pour les evenements
     */

    // gestion des appels entrants
    GSM::ExternalEvents::onIncommingCall = []()
    {
        eventHandlerApp.setTimeout(new Callback<>([](){AppManager::get(".receivecall")->run(false);}), 0);
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

    /*
    for(auto c : cc) {
        //std::cout << c.name << " " << c.phone << std::endl;
    }
    */


    /**
     * Gestion des applications
     */
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
