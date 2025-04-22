#ifdef ESP_PLATFORM
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <esp_log.h>

#include <backtrace_saver.hpp>
#include <backtrace.hpp>

#include <Arduino.h>

SET_LOOP_TASK_STACK_SIZE(8 * 1024);
#endif

#include <unistd.h>

#include <graphics.hpp>
#include <hardware.hpp>
#include <path.hpp>
#include <threads.hpp>
#include <lua_file.hpp>
#include <gsm.hpp>
#include <app.hpp>
#include <contacts.hpp>
#include <FileConfig.hpp>
#include <libsystem.hpp>
#include <GuiManager.hpp>
#include <standby.hpp>

static void launchOOBEApp() {
    try {
        const std::shared_ptr<AppManager::App> oobeApp = AppManager::get(".oobe");

        oobeApp->run();
    } catch (std::runtime_error& e) {
        libsystem::log("Unable to start OOBE application: " + std::string(e.what()));
    }
}

static bool initGraphics() {
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
    return true;
}

static bool initBattery() {
    if (GSM::getBatteryLevel() < 0.05 && !hardware::isCharging()) {
        libsystem::registerBootError("Battery level is too low.");
        libsystem::registerBootError(std::to_string(static_cast<int>(GSM::getBatteryLevel() * 100)) + "% < 5%");
        libsystem::registerBootError("Please charge your Paxo.");
        libsystem::registerBootError("Tip: Force boot by plugging a charger.");
        libsystem::displayBootErrors();
        return false;
    }
    return true;
}

static bool initStorage() {
    if (!storage::init()) {
        libsystem::registerBootError("Storage initialization error.");
        libsystem::registerBootError("Please check the SD Card.");
        return false;
    }
    return true;
}

static void initBacktraceSaver() {
#ifdef ESP_PLATFORM
    backtrace_saver::init();
    libsystem::log("Backtrace: " + std::string(backtrace_saver::getBacktraceMessage()));
    backtrace_saver::backtraceEventId = eventHandlerBack.addEventListener(
        new Condition<>(&backtrace_saver::shouldSaveBacktrace),
        new Callback<>(&backtrace_saver::saveBacktrace)
    );
#endif // ESP_PLATFORM
}

static void applySystemConfiguration() {
    libsystem::FileConfig systemConfig = libsystem::getSystemConfig();

    libsystem::log("[STARTUP]: Config loaded");
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
        COLOR_WHITE = systemConfig.get<uint16_t>("settings.color.background");
    }
    graphics::setBrightness(systemConfig.get<uint8_t>("settings.brightness"));
}

/**
 * Hardware initialization process.
 */
static void initPhase1() {
    libsystem::log("[Init] Begin Phase 1: Hardware initialization.");
    hardware::init();
    libsystem::log("[Init] {Phase 1} Hardware initialized.");
    hardware::setScreenPower(true);
    libsystem::log("[Init] {Phase 1} Screen powered on.");
    initGraphics();
    libsystem::log("[Init] {Phase 1} Graphics initialized.");
    initBattery();
    libsystem::log("[Init] {Phase 1} Battery initialized.");
    setDeviceMode(libsystem::NORMAL);
    libsystem::log("[Init] {Phase 1} Device restored NORMAL mode.");
    initStorage();
    libsystem::log("[Init] {Phase 1} Storage initialized.");
    initBacktraceSaver();
    libsystem::log("[Init] {Phase 1} BacktraceSaver initialized.");
    ThreadManager::init();
    libsystem::log("[Init] {Phase 1} ThreadManager initialized.");
    libsystem::init();
    libsystem::log("[Init] {Phase 1} libsystem initialized.");
    applySystemConfiguration();
    libsystem::log("[Init] {Phase 1} System configuration applied.");
    applications::launcher::init();
    libsystem::log("[Init] {Phase 1} Setting-up launcher.");
    if (libsystem::hasBootErrors()) {
        libsystem::log("[Init] {Phase 1} Boot errors found.");
        libsystem::displayBootErrors();
        libsystem::restart(true, 10000);
        return;
    }
    libsystem::log("[Init] {Phase 1} No boot errors.");
    libsystem::log("[Init] Phase 1 done.");
}

/**
 * Software initialization process.
 */
static void initPhase2() {
    libsystem::log("[Init] Begin Phase 2: Software initialization.");
    GSM::ExternalEvents::onIncommingCall = [] {
        eventHandlerApp.setTimeout(new Callback<>([](){AppManager::get(".receivecall")->run();}), 0);
    };
    libsystem::log("[Init] {Phase 2} Incoming call handler initialized.");
    GSM::ExternalEvents::onNewMessage = [] {
#ifdef ESP_PLATFORM
        eventHandlerBack.setTimeout(new Callback<>([](){hardware::vibrator::play({1, 0, 1});}), 0);
#endif
        AppManager::event_onmessage();
    };
    libsystem::log("[Init] {Phase 2} Incoming message handler initialized.");
    GSM::ExternalEvents::onNewMessageError = [] {
        AppManager::event_onmessageerror();
    };
#ifdef ESP_PLATFORM
    ThreadManager::new_thread(CORE_BACK, &hardware::vibrator::thread, 2*1024);
#endif
    libsystem::log("[Init] {Phase 2} Incoming message error handler initialized.");
    eventHandlerBack.setInterval(
        &graphics::touchUpdate,
        10
    );
    libsystem::log("[Init] {Phase 2} Touch handler initialized.");
    libsystem::log("[Init] Phase 2 Done.");
}

/**
 * Setup phase.
 */
static void initPhase3() {
    libsystem::log("[Init] Begin Phase 3: Setup.");
    hardware::setVibrator(false);
    libsystem::log("[Init] {Phase 3} Vibrator reset.");
    GSM::endCall();
    libsystem::log("[Init] {Phase 3} Call status reset.");
    eventHandlerApp.setTimeout(new Callback<>([](){Contacts::load();}), 0);
    libsystem::log("[Init] {Phase 3} Contacts loaded.");
    AppManager::init();
    libsystem::log("[Init] {Phase 3} AppManager initialized.");
    hardware::vibrator::play({true, true, false, false, true, false, true});
    libsystem::log("[Init] {Phase 3} Vibrator sequence played.");
    libsystem::log("[Init] Phase 3 done.");
}

static bool updateHomeScreen(GuiManager &guiManager) {
    bool appLaunched = false;

    if(applications::launcher::iconTouched())
    {
        const std::shared_ptr<AppManager::App> app = applications::launcher::getApp();

        applications::launcher::free();
        appLaunched = true;
        try {
            app->run();
        } catch (std::runtime_error& e) {
            libsystem::log("Unable to start app: " + std::string(e.what()));
            guiManager.showErrorMessage(e.what());
        }
    }
    return appLaunched;
}

static void quitAllApplications() {
    for (uint32_t i = 0; i < 10 && AppManager::isAnyVisibleApp(); i++)
        AppManager::quitApp();
}

static void goToSleep() {
    quitAllApplications();
    setDeviceMode(libsystem::SLEEP);
    StandbyMode::enable();
}

static void wakeFromSleep() {
    setDeviceMode(libsystem::NORMAL);
    StandbyMode::disable();
#ifndef ESP_PLATFORM
    applications::launcher::draw();
#endif
}

void mainLoop([[maybe_unused]] void* data) {
    GuiManager& guiManager = GuiManager::getInstance();
    const libsystem::FileConfig systemConfig = libsystem::getSystemConfig();
    bool inLauncher = false;

    libsystem::log("[STARTUP]: run mainLoop");
#ifdef ESP_PLATFORM
    if (!backtrace_saver::isBacktraceEmpty())
        backtrace_saver::backtraceMessageGUI();
    libsystem::setDeviceMode(libsystem::NORMAL);
#endif
    if (!systemConfig.has("oobe") || !systemConfig.get<bool>("oobe"))
        launchOOBEApp();
    while (true) {
        hardware::input::update();
        AppManager::loop();
        eventHandlerApp.update();
        if (AppManager::isAnyVisibleApp() && inLauncher) {
            applications::launcher::free();
            inLauncher = false;
        }
        if (inLauncher)
            applications::launcher::update();
        if (libsystem::getDeviceMode() == libsystem::NORMAL && !AppManager::isAnyVisibleApp()) {
            if (!inLauncher) {
                applications::launcher::init();
                inLauncher = true;
            } else
                inLauncher = !updateHomeScreen(guiManager);
        }
        if (getButtonDown(hardware::input::HOME)) {
            if(libsystem::getDeviceMode() == libsystem::SLEEP)
                wakeFromSleep();
            else if (inLauncher)
                goToSleep();
            else if(AppManager::isAnyVisibleApp())
                AppManager::quitApp();
        }
        if (libsystem::getDeviceMode() == libsystem::SLEEP && AppManager::isAnyVisibleApp())
            wakeFromSleep();
        StandbyMode::wait();
    }
}

void init([[maybe_unused]] void* data) {
    initPhase1();
    initPhase2();
    initPhase3();
    mainLoop(nullptr);
}

void setup() {
    init(nullptr);
    // ThreadManager::new_thread(CORE_APP, &init, 8*1024);
}

void loop() {}

#ifndef ESP_PLATFORM
int main(int argc, char **argv) {
    graphics::SDLInit(setup);
}
#endif
