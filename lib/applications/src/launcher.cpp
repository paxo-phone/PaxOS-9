#include <launcher.hpp>

#include <graphics.hpp>
#include <ElementBase.hpp>
#include <app.hpp>
#include <gsm.hpp>
#include <libsystem.hpp>
#include <memory>
#include <gui.hpp>
#include <GuiManager.hpp>
#include <standby.hpp>
#include <threads.hpp>


/**
 * Helper fonction
 * récupére l'heure du device (ou la locale)
 * et la renvoie au format "DDDD DD MMMM"
 */
std::string getFormatedDate()
{
    uint16_t day_ = GSM::days;
    uint16_t day = GSM::days;
    uint16_t month = GSM::months;
    uint16_t year = GSM::years;

    std::string dayName = daysOfWeek[(day+=month<3?year--:year-2,23*month/9+day+4+year/4-year/100+year/400)%7];
    std::string monthName = daysOfMonth[month==0?1:(month-1)];

    return dayName + " " + std::to_string(GSM::days) + " " + monthName;
}

std::string getBatteryIconFilename() {
    const bool isCharging = hardware::isCharging();
    const double batteryLevel = GSM::getBatteryLevel();

    if (batteryLevel < 0.2) {
        return isCharging ? "battery_charging_full" : "battery_0_bar";
    }
    if (batteryLevel < 0.3) {
        return isCharging ? "battery_charging_20" : "battery_1_bar";
    }
    if (batteryLevel < 0.5) {
        return isCharging ? "battery_charging_30" : "battery_2_bar";
    }
    if (batteryLevel < 0.6) {
        return isCharging ? "battery_charging_50" : "battery_3_bar";
    }
    if (batteryLevel < 0.7) {
        return isCharging ? "battery_charging_60" : "battery_4_bar";
    }
    if (batteryLevel < 0.8) {
        return isCharging ? "battery_charging_80" : "battery_5_bar";
    }
    if (batteryLevel < 0.9) {
        return isCharging ? "battery_charging_90" : "battery_6_bar";
    }

    return "battery_full";
}

namespace applications::launcher {
    std::shared_ptr<Window> launcherWindow = nullptr;
    std::map<gui::ElementBase*, std::shared_ptr<AppManager::App>> applicationsIconsMap;
    std::shared_ptr<AppManager::App> targetApp = nullptr;

    bool allocated = false;
    bool dirty = true;

    Label* clockLabel = nullptr;
    Label* dateLabel = nullptr;
    Label* batteryLabel = nullptr;
    Image* batteryIcon = nullptr;
    Box* chargingPopupBox = nullptr;
    Box* brightnessSliderBox = nullptr;
    Label* networkLabel = nullptr;

    Box *flightModeBox = nullptr;
    Label* flightModeText = nullptr;
    Switch* flightModeSwitch = nullptr;
    Button* flightModeButton = nullptr;

    uint64_t lastClockUpdate = 0;
    uint64_t lastBatteryUpdate = 0;

    uint64_t chargingStartTime = 0;
}

void applications::launcher::init() {
    launcherWindow = std::make_shared<Window>();
    targetApp = nullptr;
}

void applications::launcher::update() {
    // std::cout << "Launcher update" << std::endl;

    if (dirty) {
        // If dirty, free to force redraw it
        free();
        std::cout << "Launcher free" << std::endl;
    }

    if (!allocated) {
        // If launcher has been freed, redraw it
        draw();
        //std::cout << "Launcher redraw" << std::endl;
    }

    // Update dynamic elements
    // Do this before updating the window (so drawing it)
    // Because it can cause weird "blinking" effects

    //std::cout << "launcher::update 1" << std::endl;

    {
        static int min;

        if(min != GSM::minutes) {
            clockLabel->setText(std::to_string(GSM::hours) + ":" + (GSM::minutes<=9 ? "0" : "") + std::to_string(GSM::minutes));
            dateLabel->setText(getFormatedDate());

            min = GSM::minutes;
        }
    }

    //std::cout << "launcher::update 2" << std::endl;

    {
        static double lastBattery = GSM::getBatteryLevel();
        if(lastBattery != GSM::getBatteryLevel())
        {
            batteryLabel->setText(std::to_string(static_cast<int>(GSM::getBatteryLevel() * 100)) + "%");

            lastBattery = GSM::getBatteryLevel();
        }
    }

    {
        if(clockLabel->isTouched())
        {
            flightModeBox->enable();
            clockLabel->disable();
            dateLabel->disable();
        }
        if(flightModeButton->isTouched())
        {
            flightModeBox->disable();
            clockLabel->enable();
            dateLabel->enable();
        }

        if(flightModeSwitch->isTouched())
        {
            GSM::setFlightMode(flightModeSwitch->getState());
        }
    }


    //std::cout << "launcher::update 3" << std::endl;

    if (hardware::isCharging()) {
        if (chargingStartTime == 0) {
            chargingStartTime = millis();
        }

        if (chargingStartTime + 2000 > millis()) {
            chargingPopupBox->enable();
        } else {
            chargingPopupBox->disable();
        }
    } else {
        chargingStartTime = 0;
        chargingPopupBox->disable();
    }

    {
        static int lastNetwork = GSM::getNetworkStatus();
        if(lastNetwork != GSM::getNetworkStatus())
        {
            if(GSM::getNetworkStatus() == 99)
                networkLabel->setText("X");
            else
                networkLabel->setText(std::to_string((int) GSM::getNetworkStatus() * 100 / 31) + "%");

            lastNetwork = GSM::getNetworkStatus();
        }
    }

    //libsystem::log("launcher::update -");

    // Update, draw AND update touch events
    if (launcherWindow != nullptr) {
        launcherWindow->updateAll();
    }

    // Check touch events

    if (brightnessSliderBox->isFocused(true)) {
        // TODO: Refactoring

        libsystem::log("Brightness: " + graphics::brightness);

        graphics::brightness = (325 - (gui::ElementBase::touchY - 77)) * 255 / 325;
        graphics::brightness = std::clamp(
            graphics::brightness,
            static_cast<int16_t>(3),
            static_cast<int16_t>(255)
        );

        graphics::setBrightness(graphics::brightness);
    }


    for (const auto& [icon, app] : applicationsIconsMap) {
        if (icon->isTouched()) {
            targetApp = nullptr;
            std::cout << "launcher::update - touched: " << app->name << std::endl;
            targetApp = app;
        }
    }
}

void applications::launcher::draw() {
    libsystem::log("applications::launcher::draw");

    if (launcherWindow == nullptr) {
        launcherWindow = std::make_shared<Window>();
    }

    //std::cout << "launcher::update 1.1" << std::endl;

    StandbyMode::triggerPower();

    // Clock
    clockLabel = new Label(86, 42, 148, 41);
    clockLabel->setText(std::to_string(GSM::hours) + ":" + (GSM::minutes<=9 ? "0" : "") + std::to_string(GSM::minutes));    // hour
    clockLabel->setVerticalAlignment(Label::Alignement::CENTER);
    clockLabel->setHorizontalAlignment(Label::Alignement::CENTER);
    clockLabel->setFontSize(36);
    launcherWindow->addChild(clockLabel);


    //std::cout << "launcher::update 1.2" << std::endl;

    // Date
    dateLabel = new Label(55, 89, 210, 18);
    dateLabel->setText(getFormatedDate());
    dateLabel->setVerticalAlignment(Label::Alignement::CENTER);
    dateLabel->setHorizontalAlignment(Label::Alignement::CENTER);
    dateLabel->setFontSize(16);
    launcherWindow->addChild(dateLabel);


//    std::cout << "launcher::update 1.3" << std::endl;

    // Battery icon
    const auto batteryIconDarkPath = storage::Path("system/icons/dark/" + getBatteryIconFilename() + "_64px.png");
    batteryIcon = new Image(batteryIconDarkPath, 290, 2, 32, 32, TFT_WHITE);
    batteryIcon->load();
    launcherWindow->addChild(batteryIcon);


    //std::cout << "launcher::update 1.4" << std::endl;

    // Battery label
    batteryLabel = new Label(255, 10, 40, 18);
    batteryLabel->setText(std::to_string(static_cast<int>(GSM::getBatteryLevel() * 100)) + "%");
    batteryLabel->setVerticalAlignment(Label::Alignement::CENTER);
    batteryLabel->setHorizontalAlignment(Label::Alignement::RIGHT);
    batteryLabel->setFontSize(18);
    launcherWindow->addChild(batteryLabel);


    //std::cout << "launcher::update 1.5" << std::endl;

    
    {   // Network
        networkLabel = new Label(2, 2, 30, 18);
        if(GSM::getNetworkStatus() == 99)
            networkLabel->setText("X");
        else
            networkLabel->setText(std::to_string((int) GSM::getNetworkStatus() * 100 / 31) + "%");
        networkLabel->setVerticalAlignment(Label::Alignement::CENTER);
        networkLabel->setHorizontalAlignment(Label::Alignement::CENTER);
        networkLabel->setFontSize(18);
        launcherWindow->addChild(networkLabel);
    }

    {   // Flight mode
        flightModeBox = new Box(50, 25, 220, 59);
        flightModeBox->setBorderColor(0xCF3D);
        flightModeBox->setBorderSize(1);
        flightModeBox->setRadius(17);
        flightModeBox->disable();
        launcherWindow->addChild(flightModeBox);
    
        flightModeText = new Label(14, 8, 46, 39);
            flightModeText->setText("Mode\nAvion");
            flightModeText->setVerticalAlignment(Label::Alignement::CENTER);
            flightModeText->setFontSize(14);
        flightModeBox->addChild(flightModeText);

        flightModeSwitch = new Switch(89, 19);
            flightModeSwitch->setState(GSM::isFlightMode());
        flightModeBox->addChild(flightModeSwitch);

        flightModeButton = new Button(169, 10, 38, 38);
            flightModeButton->setText("OK");
        flightModeBox->addChild(flightModeButton);
    }

    //std::cout << "launcher::update 1.6" << std::endl;

    // Brightness slider
    brightnessSliderBox = new Box(0, 77, 40, 325);
    launcherWindow->addChild(brightnessSliderBox);

    /**
     * Gestion de l'affichage des applications
     */
    std::vector<gui::ElementBase*> apps;


    // List contenant les app
    VerticalList* winListApps = new VerticalList(0, 164, 320,316);
    //winListApps->setBackgroundColor(COLOR_GREY);
    launcherWindow->addChild(winListApps);

    // Placement des app dans l'écran
    int placementIndex = 0;

    for (const auto& app : AppManager::appList) {
        if (!app->visible) {
            // If an app is not visible (AKA. Background app)
            // Skip it

            continue;
        }

//        Box* box = new Box(60 + 119 * (placementIndex%2), 164 + 95 * int(placementIndex/2), 80, 80);
        auto* box = new Box(60 + 119 * (placementIndex%2), 95 * (placementIndex / 2), 80, 80);

        auto* img = new Image(app->path / "../icon.png", 20, 6, 40, 40);
        img->load();
        box->addChild(img);

        auto* text = new Label(0, 46, 80, 34);
        text->setText(app->name);
        text->setVerticalAlignment(Label::Alignement::CENTER);
        text->setHorizontalAlignment(Label::Alignement::CENTER);
        text->setFontSize(16);
        box->addChild(text);

        if(storage::Path notifs = (app->path / ".." / "unread.txt"); notifs.exists()) {
            storage::FileStream file(notifs.str(), storage::READ);

            if(file.size() > 0) {
                auto* notifBox = new Box(66, 0, 14, 14);
                notifBox->setRadius(7);
                notifBox->setBackgroundColor(COLOR_WARNING);
                box->addChild(notifBox);
            }

            file.close();
        }

        winListApps->addChild(box);

        applicationsIconsMap.insert({
            box,
            app
        });

        placementIndex++;
    }

    // "Overlay"
    chargingPopupBox = new Box(112, 192, 96, 96);
    chargingPopupBox->setRadius(7);
    chargingPopupBox->setBackgroundColor(TFT_BLACK);

    const auto batteryIconLightPath = storage::Path("system/icons/light/" + getBatteryIconFilename() + "_64px.png");
    const auto chargingIconImage = new Image(
        batteryIconLightPath,
        16,
        16,
        64,
        64,
        TFT_BLACK
    );
    chargingIconImage->load(TFT_BLACK);
    chargingPopupBox->addChild(chargingIconImage);

    launcherWindow->addChild(chargingPopupBox);

    // Update variables
    allocated = true;
    dirty = false;

    lastClockUpdate = millis();
    lastBatteryUpdate = millis();
}

bool applications::launcher::iconTouched() {
    return targetApp != nullptr;
}

std::shared_ptr<AppManager::App> applications::launcher::getApp() {
    return targetApp;
}

void applications::launcher::free() {
    if (!allocated) {
        return;
    }

    if (launcherWindow != nullptr) {
        launcherWindow->free();
        launcherWindow.reset();
        launcherWindow = nullptr;
    }

    applicationsIconsMap.clear();

    clockLabel = nullptr;
    dateLabel = nullptr;
    chargingPopupBox = nullptr;

    allocated = false;
    dirty = true;
}
