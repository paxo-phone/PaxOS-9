#ifdef ESP_PLATFORM

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include <backtrace_saver.hpp>
#include <backtrace.hpp>

#include <Arduino.h>

SET_LOOP_TASK_STACK_SIZE(12 * 1024);

#endif

#include <unistd.h>

#include <graphics.hpp>
#include <hardware.hpp>
#include <gui.hpp>
#include <path.hpp>
#include <threads.hpp>
#include <lua_file.hpp>
#include <gsm2.hpp>
#include <app.hpp>
#include <contacts.hpp>
#include <FileConfig.hpp>
#include <iostream>
#include <libsystem.hpp>
#include <GuiManager.hpp>
#include <standby.hpp>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#endif
#include "unistd.h"
#include <SerialManager.hpp>
#include "../lib/tasks/src/delay.hpp"

using namespace gui::elements;

void mainLoop(void* data) {
    libsystem::log("[STARTUP]: run mainLoop");
#ifdef ESP_PLATFORM
    if (!backtrace_saver::isBacktraceEmpty()) {
        backtrace_saver::backtraceMessageGUI();
    }

    //libsystem::setDeviceMode(libsystem::NORMAL);
#endif

    GuiManager& guiManager = GuiManager::getInstance();

    // ReSharper disable once CppTooWideScopeInitStatement
    const libsystem::FileConfig systemConfig = libsystem::getSystemConfig();

    // TODO: Load launcher before OOBE app, to make the experience smoother.

    // Check if OOBE app need to be launched
    if (!systemConfig.has("oobe") || !systemConfig.get<bool>("oobe")) {
        // Launch OOBE app
        try {
            const std::shared_ptr<AppManager::App> oobeApp = AppManager::get(".oobe");

            if (oobeApp == nullptr) {
                throw std::runtime_error("OOBE app not found.");
            }

            oobeApp->run();
        } catch (std::runtime_error& e) {
            //std::cerr << "Lua error: " << e.what() << std::endl;
            //guiManager.showErrorMessage(e.what());
            //AppManager::appList[i].kill();
        }
    }

    bool launcher = false;
    while (true)    // manage the running apps, the launcher and the sleep mode
    {
        hardware::input::update();
        AppManager::loop();
        eventHandlerApp.update();

        if(AppManager::isAnyVisibleApp() && launcher)   // free the launcher is an app is running and the launcher is active
        {
            applications::launcher::free();
            launcher = false;
        }

        if(launcher)
            applications::launcher::update();


        if(libsystem::getDeviceMode() == libsystem::NORMAL && !AppManager::isAnyVisibleApp())   // si mode normal et pas d'app en cours
        {
            if(!launcher)   // si pas de launcher -> afficher un launcher
            {
                applications::launcher::init();
                launcher = true;
            }
            else    // si launcher -> l'update et peut être lancer une app
            {
                if(applications::launcher::iconTouched())
                {
                    // run the app
                    const std::shared_ptr<AppManager::App> app = applications::launcher::getApp();

                    // Free the launcher resources
                    applications::launcher::free();
                    launcher = false;

                    // Launch the app
                    try {
                        app->run();
                    } catch (std::runtime_error& e) {
                        std::cerr << "Erreur: " << e.what() << std::endl;
                        // Affichage du msg d'erreur
                        guiManager.showErrorMessage(e.what());
                    }
                }
            }
        }

        if(hardware::getHomeButton())    // si on appuie sur HOME
        {
            while(hardware::getHomeButton());

            if(libsystem::getDeviceMode() == libsystem::SLEEP)
            {
                setDeviceMode(libsystem::NORMAL);
                StandbyMode::disable();

                #ifndef ESP_PLATFORM
                applications::launcher::draw();
                #endif
            } else if(launcher && !AppManager::didRequestAuth)
            {
                libsystem::setDeviceMode(libsystem::SLEEP);
                StandbyMode::enable();
                continue;
            } else if(AppManager::isAnyVisibleApp())
            {
                AppManager::quitApp();
            } else if (AppManager::didRequestAuth)
            {
                AppManager::didRequestAuth = false;
            }
        }

        if(libsystem::getDeviceMode() == libsystem::SLEEP && AppManager::isAnyVisibleApp())
        {
            setDeviceMode(libsystem::NORMAL);
            StandbyMode::disable();
        }


        if(libsystem::getDeviceMode() != libsystem::SLEEP && StandbyMode::expired())    // innactivity detected -> go to sleep mode
        {
            for (uint32_t i = 0; i < 10 && AppManager::isAnyVisibleApp(); i++)  // define a limit on how many apps can be stopped (prevent from a loop)
            {
                AppManager::quitApp();
            }
            libsystem::setDeviceMode(libsystem::SLEEP);
            StandbyMode::enable();
        }

        if(libsystem::getDeviceMode() == libsystem::SLEEP)
            StandbyMode::sleepCycle();
        else
            StandbyMode::wait();

        /*std::cout << "states: "
                  << "StandbyMode: " << (StandbyMode::state() ? "enabled" : "disabled")
                  << ", deviceMode: " << (libsystem::getDeviceMode() == libsystem::NORMAL ? "normal" : "sleep")
                  << ", anyVisibleApp: " << (AppManager::isAnyVisibleApp() ? "true" : "false")
                  << std::endl;*/
    }
}

static bool initGraphics()
{
    if (const graphics::GraphicsInitCode graphicsInitCode = graphics::init(); graphicsInitCode != graphics::SUCCESS) {
        libsystem::registerBootError("Graphics initialization error.");

        if (graphicsInitCode == graphics::ERROR_NO_TOUCHSCREEN) {
            libsystem::registerBootError("No touchscreen found.");
        } else if (graphicsInitCode == graphics::ERROR_FAULTY_TOUCHSCREEN) {
            libsystem::registerBootError("Faulty touchscreen detected.");
        }
        return false;
    }
    setScreenOrientation(graphics::PORTRAIT);
    libsystem::info("Graphics initialized successfully.");
    return true;
}

static bool initStorage()
{
    if (!storage::init()) {
        libsystem::registerBootError("Storage initialization error.");
        libsystem::registerBootError("Please check the SD Card.");
        return false;
    }
    libsystem::info("[STARTUP]: Storage initialized");
    return true;
}

static bool initLibSystem()
{
    libsystem::init();
    libsystem::FileConfig systemConfig = libsystem::getSystemConfig();

    if (!systemConfig.has("settings.brightness")) {
        systemConfig.set<uint8_t>("settings.brightness", 69);
        systemConfig.write();
    }
    if (!systemConfig.has("settings.sleeptime")) {
        systemConfig.set<uint64_t>("settings.sleeptime", 30000);
        systemConfig.write();
    }
    if (!systemConfig.has("settings.color.background")) {
        libsystem::paxoConfig::setBackgroundColor(0xFFFF, true);
    } else {
        // FIXME: That's not the right way to do it, but it works for now.
        COLOR_WHITE = systemConfig.get<uint16_t>("settings.color.background");
    }
    graphics::setBrightness(systemConfig.get<uint8_t>("settings.brightness"));
    libsystem::info("LibSystem initialized successfully.");
    return true;
}

static bool registerEventHandlers() {
    Gsm::ExternalEvents::onIncommingCall = [] {
        eventHandlerApp.setTimeout(
            new Callback<>([] {
                AppManager::get(".receivecall")->run();
            }),
            0
        );
    };
    Gsm::ExternalEvents::onNewMessage = [] {
#ifdef ESP_PLATFORM
        eventHandlerBack.setTimeout(new Callback<>([] {
            hardware::vibrator::play({1, 0, 1});
        }), 0);
#endif
        AppManager::event_onmessage();
    };
    Gsm::ExternalEvents::onNewMessageError = [] {
        AppManager::event_onmessageerror();
    };
    eventHandlerBack.setInterval(
        &graphics::touchUpdate,
        10
    );
    return true;
}

void init(void* data)
{
    ThreadManager::init();
#ifdef ESP_PLATFORM
    ThreadManager::new_thread(CORE_BACK, &serialcom::SerialManager::serialLoop);
    ThreadManager::new_thread(CORE_BACK, &hardware::vibrator::thread, 2*1024);
#endif // ESP_PLATFORM

    hardware::init();
    hardware::setScreenPower(true);

    libsystem::info("Hardware initialized.");

    if (!initGraphics()) {
        libsystem::displayBootErrors();
        libsystem::restart(true, 10000);
        return;
    }

    if (Gsm::getBatteryLevel() < 0.0) {
        libsystem::registerBootError("Battery error.");
        libsystem::displayBootErrors();
        libsystem::restart(true, 10000);
        return;
    }

    if (!initStorage()) {
        libsystem::displayBootErrors();
        libsystem::restart(true, 10000);
        return;
    }

#ifdef ESP_PLATFORM
    backtrace_saver::init();
    libsystem::info("Backtrace: " + backtrace_saver::getBacktraceMessage());
    backtrace_saver::backtraceEventId = eventHandlerBack.addEventListener(
        new Condition<>(&backtrace_saver::shouldSaveBacktrace),
        new Callback<>(&backtrace_saver::saveBacktrace)
    );
#endif // ESP_PLATFORM

    initLibSystem();

    applications::launcher::init();

    registerEventHandlers();

    hardware::setVibrator(false);
    //GSM::endCall();

    eventHandlerApp.setTimeout(new Callback<>([](){Contacts::load();}), 0);

    libsystem::info("Loaded contacts.");

    AppManager::init();

    hardware::vibrator::play({
        true,
        true,
        false,
        false,
        true,
        false,
        true
    });

    libsystem::info("Initialization completed.");

    mainLoop(nullptr);
}

void setup()
{
#ifdef ESP_PLATFORM
    esp_task_wdt_init(5000, true);
#endif
    
    init(nullptr);
}

void loop()
{}

#ifndef ESP_PLATFORM

int main(int argc, char **argv)
{
    graphics::SDLInit(setup);
}

#endif
