#ifdef ESP_PLATFORM

#include <Arduino.h>
#include <backtrace.hpp>
#include <backtrace_saver.hpp>
#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

SET_LOOP_TASK_STACK_SIZE(4 * 1024);

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
#include <network.hpp>
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

/**
 * @brief The main loop of the system.
 *
 * This function orchestrates the entire device's lifecycle, including:
 * - Initial setup and configuration.
 * - Running the Out-of-Box Experience (OOBE) on first launch.
 * - Managing the application lifecycle (launching, running, quitting).
 * - Handling the launcher UI.
 * - Managing the device's power states (NORMAL, SLEEP).
 * - Responding to hardware button presses.
 *
 * @param data A pointer to any data that might be passed to the main loop thread.
 */
void mainLoop(void* data)
{
    libsystem::log("[STARTUP]: run mainLoop");

#ifdef ESP_PLATFORM
    // On ESP platforms, check for and display any saved crash backtraces.
    if (!backtrace_saver::isBacktraceEmpty())
        backtrace_saver::backtraceMessageGUI();
#endif

    // Get a reference to the singleton GuiManager instance.
    GuiManager& guiManager = GuiManager::getInstance();

    // Load the system configuration from a file.
    const libsystem::FileConfig systemConfig = libsystem::getSystemConfig();

    // Check if the Out-of-Box Experience (OOBE) app needs to be run.
    // This is typically done on the first boot or after a factory reset.
    if (!systemConfig.has("oobe") || !systemConfig.get<bool>("oobe"))
    {
        try
        {
            // Find the OOBE app using the AppManager.
            const std::shared_ptr<AppManager::App> oobeApp = AppManager::get(".oobe");

            if (oobeApp == nullptr)
                throw std::runtime_error("OOBE app not found.");

            // Run the OOBE app. This is a blocking call until the app finishes.
            oobeApp->run();
        }
        catch (std::runtime_error& e)
        {
            // If the OOBE app fails to launch or crashes, show an error message.
            guiManager.showErrorMessage(e.what());
        }
    }

    // Flag to track if the launcher is currently active and initialized.
    bool launcher = false;

    // This is the main event loop that runs for the lifetime of the device.
    while (true)
    {
        // Poll for hardware input events (e.g., button presses, touch screen).
        hardware::input::update();
        // Allow the AppManager to perform its own loop tasks (e.g., IPC, state checks).
        AppManager::loop();
        // Update the event handler application.
        eventHandlerApp.update();

        // If an application is running and visible, the launcher is not needed.
        // This block frees the launcher's resources to save memory.
        if (AppManager::isAnyVisibleApp() && launcher)
        {
            applications::launcher::free();
            launcher = false;
        }

        // If the launcher is active, update its state (e.g., handle UI animations, touch events).
        if (launcher)
            applications::launcher::update();

        // If we are in NORMAL mode and no app is visible, we should display the launcher.
        if (libsystem::getDeviceMode() == libsystem::NORMAL && !AppManager::isAnyVisibleApp())
        {
            // If the launcher isn't already active, initialize it.
            if (!launcher)
            {
                applications::launcher::init();
                launcher = true;
            }
            // If the launcher is active, check if an app icon has been touched.
            else if (applications::launcher::iconTouched())
            {
                // Get the application associated with the touched icon.
                const std::shared_ptr<AppManager::App> app = applications::launcher::getApp();

                // Free launcher resources before launching the app.
                applications::launcher::free();
                launcher = false;

                // Launch the selected app.
                try
                {
                    app->run();
                }
                catch (std::runtime_error& e)
                {
                    // If the app fails to run, display an error message.
                    std::cerr << "Erreur: " << e.what() << std::endl;
                    guiManager.showErrorMessage(e.what());
                }
            }
        }

        if (hardware::getHomeButton())
        {
            // Wait for the button to be released to debounce it.
            uint32_t debounceTime = os_millis();
            while (hardware::getHomeButton() && debounceTime + 1000 > os_millis())
            {
                PaxOS_Delay(1);
            }

            if(debounceTime + 1000 <= os_millis())   // show the power menu if the button was held for more than 1 second
            {
                while (hardware::getHomeButton());
                PaxOS_Delay(10);

                auto& win = guiManager.getWindow();
                win.setBackgroundColor(COLOR_DARK);
                
                Label* label = new Label(
                    0, 200, 320, 150
                );
                label->setText("Toucher ici pour éteindre le paxo\n(Ou appuyer sur le bouton menu pour annuler)");
                label->setTextColor(COLOR_WHITE);
                label->setBackgroundColor(COLOR_DARK);
                label->setFontSize(30);
                label->setHorizontalAlignment(Label::Alignement::CENTER);
                win.addChild(label);

                while (true)
                {
                    win.updateAll();

                    eventHandlerApp.update();
                    StandbyMode::wait();

                    if(label->isFocused(true))
                    {
                        // If the screen is touched, power off the device.
                        libsystem::poweroff();
                        // No need to continue the loop, as the device is powered off. No further actions will be taken.
                    }

                    if(hardware::getHomeButton())
                    {
                        break;
                    }
                }
            }

            // If the device is asleep, the home button wakes it up.
            if (libsystem::getDeviceMode() == libsystem::SLEEP)
            {
                setDeviceMode(libsystem::NORMAL);
                StandbyMode::disable();
                continue;
            }
            // If the launcher is active, the home button puts the device to sleep.
            else if (launcher && !AppManager::didRequestAuth)
            {
                libsystem::setDeviceMode(libsystem::SLEEP);
                StandbyMode::enable();
                // Continue to the next loop iteration to immediately start the sleep cycle.
                continue;
            }
            // If an app is visible, the home button quits the app, returning to the launcher.
            else if (AppManager::isAnyVisibleApp())
            {
                AppManager::quitApp();
            }
            // Handle cancellation of an authentication request.
            else if (AppManager::didRequestAuth)
            {
                AppManager::didRequestAuth = false;
            }
        }
        else if (libsystem::getDeviceMode() == libsystem::SLEEP && AppManager::isAnyVisibleApp())
        {
            setDeviceMode(libsystem::NORMAL);
            StandbyMode::disable();
        }
        else if (libsystem::getDeviceMode() != libsystem::SLEEP && StandbyMode::expired())
        {
            // Before sleeping, attempt to quit any visible apps.
            for (uint32_t i = 0; i < 10 && AppManager::isAnyVisibleApp(); i++)
            {
                AppManager::quitApp();
            }
            // Set the device to sleep mode.
            libsystem::setDeviceMode(libsystem::SLEEP);
            StandbyMode::enable();
        }

        if(Gsm::CallState() != Gsm::CallState::IDLE)
            StandbyMode::reset();

        // Based on the final state determined above, either perform a low-power sleep
        // cycle or wait for a short period before the next loop iteration.
        if (libsystem::getDeviceMode() == libsystem::SLEEP)
            StandbyMode::sleepCycle(); // Low-power wait
        else
            StandbyMode::wait(); // Normal wait

        // Logging for debugging the device's state at the end of each loop.
        /*std::cout << "states: "
                  << "StandbyMode: " << (StandbyMode::state() ? "enabled" : "disabled")
                  << ", deviceMode: " << (libsystem::getDeviceMode() == libsystem::NORMAL ? "normal" : "sleep")
                  << ", anyVisibleApp: " << (AppManager::isAnyVisibleApp() ? "true" : "false")
                  << std::endl;*/
    }
}

void init(void* data)
{
/**
 * Initialisation du hardware, de l'écran, lecture des applications stcokées dans storage
 */
#ifdef ESP_PLATFORM
    ThreadManager::new_thread(CORE_BACK, &serialcom::SerialManager::serialLoop, 10 * 1024);
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
        libsystem::registerBootError(std::to_string(static_cast<int>(GSM::getBatteryLevel() * 100))
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
        libsystem::log("[STARTUP]: Storage initialized");

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

    Network::init();
    libsystem::log("[STARTUP]: Network initialized");

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

#ifdef __cplusplus
extern "C"
{
#endif

    void app_main(void)
    {
#ifdef ESP_PLATFORM
        esp_task_wdt_init(5000, true);
#endif

        init(NULL);

        // An app_main function running as a FreeRTOS task should not return.
        // If init() creates other tasks and this one is done, it should delete itself.
        // For example: vTaskDelete(NULL);
    }

#ifdef __cplusplus
}
#endif

void loop() {}

#ifndef ESP_PLATFORM

// Native main
int main(int argc, char** argv)
{
    graphics::SDLInit(app_main);
}

#endif
