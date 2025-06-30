#ifdef ESP_PLATFORM

#include <Arduino.h>
#include <backtrace.hpp>
#include <backtrace_saver.hpp>
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

SET_LOOP_TASK_STACK_SIZE(12 * 1024);

#endif

#include <FileConfig.hpp>
#include <GuiManager.hpp>
#include <app.hpp>
#include <contacts.hpp>
#include <graphics.hpp>
#include <gsm2.hpp>
#include <gui.hpp>
#include <hardware.hpp>
#include <iostream>
#include <libsystem.hpp>
#include <lua_file.hpp>
#include <path.hpp>
#include <standby.hpp>
#include <threads.hpp>
#include <unistd.h>
#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#endif
#include "../lib/tasks/src/delay.hpp"
#include "unistd.h"

#include <SerialManager.hpp>

using namespace gui::elements;

void mainLoop(void* data)
{
    libsystem::log("[STARTUP]: run mainLoop");
#ifdef ESP_PLATFORM
    if (!backtrace_saver::isBacktraceEmpty())
        backtrace_saver::backtraceMessageGUI();

    // libsystem::setDeviceMode(libsystem::NORMAL);
#endif

    GuiManager& guiManager = GuiManager::getInstance();

    // ReSharper disable once CppTooWideScopeInitStatement
    const libsystem::FileConfig systemConfig = libsystem::getSystemConfig();

    // TODO: Load launcher before OOBE app, to make the experience smoother.

    // Check if OOBE app need to be launched
    if (!systemConfig.has("oobe") || !systemConfig.get<bool>("oobe"))
    {
        // Launch OOBE app
        try
        {
            const std::shared_ptr<AppManager::App> oobeApp = AppManager::get(".oobe");

            if (oobeApp == nullptr)
                throw std::runtime_error("OOBE app not found.");

            oobeApp->run();
        }
        catch (std::runtime_error& e)
        {
            // std::cerr << "Lua error: " << e.what() << std::endl;
            // guiManager.showErrorMessage(e.what());
            // AppManager::appList[i].kill();
        }
    }

    bool launcher = false;
    while (true) // manage the running apps, the launcher and the sleep mode
    {
        hardware::input::update();
        AppManager::loop();
        eventHandlerApp.update();

        if (AppManager::isAnyVisibleApp() && launcher) // free the launcher is an app is running and
                                                       // the launcher is active
        {
            applications::launcher::free();
            launcher = false;
        }

        if (launcher)
            applications::launcher::update();

        if (libsystem::getDeviceMode() == libsystem::NORMAL &&
            !AppManager::isAnyVisibleApp()) // si mode normal et pas d'app en cours
        {
            if (!launcher) // si pas de launcher -> afficher un launcher
            {
                applications::launcher::init();
                launcher = true;
            }
            else // si launcher -> l'update et peut être lancer une app
                if (applications::launcher::iconTouched())
                {
                    // run the app
                    const std::shared_ptr<AppManager::App> app = applications::launcher::getApp();

                    // Free the launcher resources
                    applications::launcher::free();
                    launcher = false;

                    // Launch the app
                    try
                    {
                        app->run();
                    }
                    catch (std::runtime_error& e)
                    {
                        std::cerr << "Erreur: " << e.what() << std::endl;
                        // Affichage du msg d'erreur
                        guiManager.showErrorMessage(e.what());
                    }
                }
        }

        if (hardware::getHomeButton()) // si on appuie sur HOME
        {
            while (hardware::getHomeButton());

            if (libsystem::getDeviceMode() == libsystem::SLEEP)
            {
                setDeviceMode(libsystem::NORMAL);
                StandbyMode::disable();

#ifndef ESP_PLATFORM
                applications::launcher::draw();
#endif
            }
            else if (launcher && !AppManager::didRequestAuth)
            {
                libsystem::setDeviceMode(libsystem::SLEEP);
                StandbyMode::enable();
                continue;
            }
            else if (AppManager::isAnyVisibleApp())
            {
                AppManager::quitApp();
            }
            else if (AppManager::didRequestAuth)
            {
                AppManager::didRequestAuth = false;
            }
        }

        if (libsystem::getDeviceMode() == libsystem::SLEEP && AppManager::isAnyVisibleApp())
        {
            setDeviceMode(libsystem::NORMAL);
            StandbyMode::disable();
        }

        if (libsystem::getDeviceMode() != libsystem::SLEEP &&
            StandbyMode::expired()) // innactivity detected -> go to sleep mode
        {
            for (uint32_t i = 0; i < 10 && AppManager::isAnyVisibleApp();
                 i++) // define a limit on how many apps can be stopped (prevent
                      // from a loop)
            {
                AppManager::quitApp();
            }
            libsystem::setDeviceMode(libsystem::SLEEP);
            StandbyMode::enable();
        }

        if (libsystem::getDeviceMode() == libsystem::SLEEP)
            StandbyMode::sleepCycle();
        else
            StandbyMode::wait();

        /*std::cout << "states: "
                  << "StandbyMode: " << (StandbyMode::state() ? "enabled" :
           "disabled")
                  << ", deviceMode: " << (libsystem::getDeviceMode() ==
           libsystem::NORMAL ? "normal" : "sleep")
                  << ", anyVisibleApp: " << (AppManager::isAnyVisibleApp() ?
           "true" : "false")
                  << std::endl;*/
    }
}

void init(void* data)
{
/**
 * Initialisation du hardware, de l'écran, lecture des applications stcokées
 * dans storage
 */
#ifdef ESP_PLATFORM
    ThreadManager::new_thread(CORE_BACK, &serialcom::SerialManager::serialLoop);
#endif
    hardware::init();
    libsystem::log("[STARTUP]: Hardware initialized");
    hardware::setScreenPower(true);

    // Init graphics and check for errors
    if (const graphics::GraphicsInitCode graphicsInitCode = graphics::init();
        graphicsInitCode != graphics::SUCCESS)
    {
        libsystem::registerBootError("Graphics initialization error.");

        if (graphicsInitCode == graphics::ERROR_NO_TOUCHSCREEN)
            libsystem::registerBootError("No touchscreen found.");
        else if (graphicsInitCode == graphics::ERROR_FAULTY_TOUCHSCREEN)
            libsystem::registerBootError("Faulty touchscreen detected.");
    }
    setScreenOrientation(graphics::PORTRAIT);

    libsystem::log("[STARTUP]: Graphics initialized");

    // If battery is too low
    // Don't initialize ANY MORE service
    // But display error
    /*if (GSM::getBatteryLevel() < 0.05 && !hardware::isCharging()) {
        libsystem::registerBootError("Battery level is too low.");
        libsystem::registerBootError(std::to_string(static_cast<int>(GSM::getBatteryLevel()
    * 100))
    + "% < 5%"); libsystem::registerBootError("Please charge your Paxo.");
        libsystem::registerBootError("Tip: Force boot by plugging a charger.");

        libsystem::displayBootErrors();

        // TODO: Set device mode to sleep

        return;
    }*/

    // Init storage and check for errors
    if (!storage::init())
    {
        libsystem::registerBootError("Storage initialization error.");
        libsystem::registerBootError("Please check the SD Card.");
    }
    else
    {
        libsystem::log("[STARTUP]: Storage initialized");
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

    libsystem::log("[STARTUP]: Threads initialized");

    libsystem::init();
    libsystem::FileConfig systemConfig = libsystem::getSystemConfig();

    libsystem::log("[STARTUP]: Config loaded");

    if (!systemConfig.has("settings.brightness"))
    {
        systemConfig.set<uint8_t>("settings.brightness", 69);
        systemConfig.write();
    }

    if (!systemConfig.has("settings.sleeptime"))
    {
        systemConfig.set<uint64_t>("settings.sleeptime", 30000);
        systemConfig.write();
    }

    if (!systemConfig.has("settings.color.background"))
        libsystem::paxoConfig::setBackgroundColor(0xFFFF, true);
    else
        COLOR_WHITE = static_cast<color_t>(systemConfig.get<uint16_t>("settings.color.background"));

    libsystem::log(
        "settings.brightness: " + std::to_string(systemConfig.get<uint8_t>("settings.brightness"))
    );

    graphics::setBrightness(systemConfig.get<uint8_t>("settings.brightness"));

    // Init launcher
    applications::launcher::init();

    // When everything is initialized
    // Check if errors occurred
    // If so, restart
    if (libsystem::hasBootErrors())
    {
        libsystem::displayBootErrors();
        libsystem::restart(true, 10000);
    }

    /**
     * Gestion des eventHandlers pour les evenements
     */

    // gestion des appels entrants
    Gsm::ExternalEvents::onIncommingCall = []()
    {
        eventHandlerApp.setTimeout(
            new Callback<>(
                []()
                {
                    AppManager::get(".receivecall")->run();
                }
            ),
            0
        );
    };

    // Gestion de la réception d'un message
    Gsm::ExternalEvents::onNewMessage = []()
    {
#ifdef ESP_PLATFORM
        eventHandlerBack.setTimeout(
            new Callback<>(
                []()
                {
                    hardware::vibrator::play({1, 0, 1});
                }
            ),
            0
        );
#endif

        AppManager::event_onmessage();
    };

    Gsm::ExternalEvents::onNewMessageError = []()
    {
        AppManager::event_onmessageerror();
    };

#ifdef ESP_PLATFORM
    ThreadManager::new_thread(CORE_BACK, &hardware::vibrator::thread, 2 * 1024);
#endif

    // gestion de la détection du toucher de l'écran
    eventHandlerBack.setInterval(&graphics::touchUpdate, 10);

    hardware::setVibrator(false);
    // GSM::endCall();

    // Chargement des contacts
    std::cout << "[Main] Loading Contacts" << std::endl;
    eventHandlerApp.setTimeout(
        new Callback<>(
            []()
            {
                Contacts::load();
            }
        ),
        0
    );

    AppManager::init();

    hardware::vibrator::play({1, 1, 0, 0, 1, 0, 1});

    mainLoop(NULL);
}

void setup()
{
#ifdef ESP_PLATFORM
    esp_task_wdt_init(5000, true);
#endif

    init(NULL);
}

void loop() {}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char** argv)
{
    graphics::SDLInit(setup);
}

#endif
