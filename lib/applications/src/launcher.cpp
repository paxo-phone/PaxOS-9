#include <ElementBase.hpp>
#include <GuiManager.hpp>
#include <app.hpp>
#include <graphics.hpp>
#include <gsm2.hpp>
#include <gui.hpp>
#include <launcher.hpp>
#include <libsystem.hpp>
#include <memory>
#include <standby.hpp>
#include <threads.hpp>

/**
 * Helper fonction
 * récupére l'heure du device (ou la locale)
 * et la renvoie au format "DDDD DD MMMM"
 */
std::string getFormatedDate()
{
    int16_t day_ = Gsm::Time::getDay();
    int16_t day = Gsm::Time::getDay();
    int16_t month = Gsm::Time::getMonth();
    int16_t year = Gsm::Time::getYear();

    if (day_ == -1)
        day_ = 1;
    if (day == -1)
        day = 1;
    if (month == -1)
        month = 1;
    if (year == -1)
        year = 1970;

    std::string dayName = daysOfWeek
        [(day += month < 3 ? year-- : year - 2,
          23 * month / 9 + day + 4 + year / 4 - year / 100 + year / 400) %
         7];
    std::string monthName = daysOfMonth[month == 0 ? 1 : (month - 1)];

    return dayName + " " + std::to_string(Gsm::Time::getDay()) + " " + monthName;
}

std::string getBatteryIconFilename()
{
    const bool isCharging = hardware::isCharging();
    const double batteryLevel =
        Gsm::getBatteryLevel(); // TODO: Replace with actual battery level calculation

    if (batteryLevel < 0.2)
        return isCharging ? "battery_charging_full" : "battery_0_bar";
    if (batteryLevel < 0.3)
        return isCharging ? "battery_charging_20" : "battery_1_bar";
    if (batteryLevel < 0.5)
        return isCharging ? "battery_charging_30" : "battery_2_bar";
    if (batteryLevel < 0.6)
        return isCharging ? "battery_charging_50" : "battery_3_bar";
    if (batteryLevel < 0.7)
        return isCharging ? "battery_charging_60" : "battery_4_bar";
    if (batteryLevel < 0.8)
        return isCharging ? "battery_charging_80" : "battery_5_bar";
    if (batteryLevel < 0.9)
        return isCharging ? "battery_charging_90" : "battery_6_bar";

    return "battery_full";
}

namespace applications::launcher
{
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

    Box* flightModeBox = nullptr;
    Label* flightModeText = nullptr;
    Switch* flightModeSwitch = nullptr;
    Button* flightModeButton = nullptr;

    uint64_t lastClockUpdate = 0;
    uint64_t lastBatteryUpdate = 0;

    uint64_t chargingStartTime = 0;
} // namespace applications::launcher

void applications::launcher::init()
{
    launcherWindow = std::make_shared<Window>();
    targetApp = nullptr;

    draw();
}

void applications::launcher::update()
{
    {
        static int min;

        if (min != Gsm::Time::getMinute())
        {
            clockLabel->setText(
                std::to_string(Gsm::Time::getHour()) + ":" +
                (Gsm::Time::getMinute() <= 9 ? "0" : "") + std::to_string(Gsm::Time::getMinute())
            );
            dateLabel->setText(getFormatedDate());

            min = Gsm::Time::getMinute();
        }
    }

    // std::cout << "launcher::update 2" << std::endl;

    {
        static double lastBattery = Gsm::getBatteryLevel();
        if (lastBattery != Gsm::getBatteryLevel())
        {
            batteryLabel->setText(
                std::to_string(static_cast<int>(Gsm::getBatteryLevel() * 100)) + "%"
            );

            lastBattery = Gsm::getBatteryLevel();
        }
    }

    {
        if (clockLabel->isTouched())
        {
            flightModeBox->enable();
            clockLabel->disable();
            dateLabel->disable();
        }
        if (flightModeButton->isTouched())
        {
            flightModeBox->disable();
            clockLabel->enable();
            dateLabel->enable();
        }

        if (flightModeSwitch->isTouched())
            Gsm::setFlightMode(flightModeSwitch->getState());
    }

    // std::cout << "launcher::update 3" << std::endl;

    if (hardware::isCharging())
    {
        if (chargingStartTime == 0)
            chargingStartTime = os_millis();

        if (chargingStartTime + 2000 > os_millis())
            chargingPopupBox->enable();
        else
            chargingPopupBox->disable();
    }
    else
    {
        chargingStartTime = 0;
        chargingPopupBox->disable();
    }

    {
        static int lastNetwork = Gsm::getNetworkQuality().first;
        if (lastNetwork != Gsm::getNetworkQuality().first)
        {
            if (Gsm::getNetworkQuality().first == 99)
                networkLabel->setText("X");
            else
                networkLabel->setText(
                    std::to_string((int) Gsm::getNetworkQuality().first * 100 / 31) + "%"
                );

            lastNetwork = Gsm::getNetworkQuality().first;
        }
    }

    // libsystem::log("launcher::update -");

    // Update, draw AND update touch events
    // printf("before\n");
    if (launcherWindow != nullptr)
        launcherWindow->updateAll();
    // printf("after\n");

    // Check touch events

    if (brightnessSliderBox->isFocused(true))
    {
        libsystem::log("Brightness: " + graphics::getBrightness());

        const int16_t newBrightness = static_cast<int16_t>(
            std::clamp((325 - (gui::ElementBase::touchY - 77)) * 255 / 325, 3, 255)
        );

        graphics::setBrightness(newBrightness);
    }

    // printf("after - 2\n");

    for (const auto& [icon, app] : applicationsIconsMap)
    {
        if (icon->isTouched())
        {
            targetApp = nullptr;
            std::cout << "launcher::update - touched: " << app->name << std::endl;
            targetApp = app;
        }
    }
}

void applications::launcher::draw()
{
    libsystem::log("applications::launcher::draw");

    if (!launcherWindow)
    {
        std::cout << "launcherWindow not present, aborting draw()" << std::endl;
        return;
    }

    // std::cout << "launcher::update 1.1" << std::endl;

    StandbyMode::triggerPower();

    // Clock
    clockLabel = new Label(86, 42, 148, 41);
    clockLabel->setText(
        std::to_string(Gsm::Time::getHour()) + ":" + (Gsm::Time::getMinute() <= 9 ? "0" : "") +
        std::to_string(Gsm::Time::getMinute())
    ); // hour
    clockLabel->setVerticalAlignment(Label::Alignement::CENTER);
    clockLabel->setHorizontalAlignment(Label::Alignement::CENTER);
    clockLabel->setFontSize(36);
    launcherWindow->addChild(clockLabel);

    // std::cout << "launcher::update 1.2" << std::endl;

    // Date
    dateLabel = new Label(55, 89, 210, 18);
    dateLabel->setText(getFormatedDate());
    dateLabel->setVerticalAlignment(Label::Alignement::CENTER);
    dateLabel->setHorizontalAlignment(Label::Alignement::CENTER);
    dateLabel->setFontSize(16);
    launcherWindow->addChild(dateLabel);

    //    std::cout << "launcher::update 1.3" << std::endl;

    // Battery icon
    const auto batteryIconDarkPath =
        storage::Path("system/icons/dark/" + getBatteryIconFilename() + "_64px.png");
    batteryIcon = new Image(batteryIconDarkPath, 290, 2, 32, 32);
    batteryIcon->load();
    launcherWindow->addChild(batteryIcon);

    // std::cout << "launcher::update 1.4" << std::endl;

    // Battery label
    batteryLabel = new Label(255, 10, 40, 18);
    batteryLabel->setText(std::to_string(static_cast<int>(Gsm::getBatteryLevel() * 100)) + "%");
    batteryLabel->setVerticalAlignment(Label::Alignement::CENTER);
    batteryLabel->setHorizontalAlignment(Label::Alignement::RIGHT);
    batteryLabel->setFontSize(18);
    launcherWindow->addChild(batteryLabel);

    // std::cout << "launcher::update 1.5" << std::endl;

    { // Network
        networkLabel = new Label(2, 2, 30, 18);
        if (Gsm::getNetworkQuality().first == 99)
            networkLabel->setText("X");
        else
            networkLabel->setText(
                std::to_string((int) Gsm::getNetworkQuality().first * 100 / 31) + "%"
            );
        networkLabel->setVerticalAlignment(Label::Alignement::CENTER);
        networkLabel->setHorizontalAlignment(Label::Alignement::CENTER);
        networkLabel->setFontSize(18);
        launcherWindow->addChild(networkLabel);
    }

    { // Flight mode
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
        flightModeSwitch->setState(Gsm::isFlightModeActive());
        flightModeBox->addChild(flightModeSwitch);

        flightModeButton = new Button(169, 10, 38, 38);
        flightModeButton->setText("OK");
        flightModeBox->addChild(flightModeButton);
    }

    // std::cout << "launcher::update 1.6" << std::endl;

    // Brightness slider
    brightnessSliderBox = new Box(0, 77, 40, 325);
    launcherWindow->addChild(brightnessSliderBox);

    /**
     * Gestion de l'affichage des applications
     */
    std::vector<gui::ElementBase*> apps;

    // List contenant les app
    VerticalList* winListApps = new VerticalList(60, 164, 320 - 60 * 2, 316);
    // winListApps->setBackgroundColor(COLOR_GREY);
    launcherWindow->addChild(winListApps);

    // Placement des app dans l'écran
    int placementIndex = 0;

    for (const auto& app : AppManager::appList)
    {
        if (!app->visible)
        {
            // If an app is not visible (AKA. Background app)
            // Skip it

            continue;
        }

        //        Box* box = new Box(60 + 119 * (placementIndex%2), 164 + 95 *
        //        int(placementIndex/2), 80, 80);
        auto* box = new Box(119 * (placementIndex % 2), 95 * (placementIndex / 2), 80, 80);

        auto* img = new Image(app->path / "../icon.png", 20, 6, 40, 40);
        img->load();
        box->addChild(img);

        auto* text = new Label(0, 46, 80, 34);
        text->setText(app->name);
        text->setVerticalAlignment(Label::Alignement::CENTER);
        text->setHorizontalAlignment(Label::Alignement::CENTER);
        text->setFontSize(16);
        box->addChild(text);

        /*if(storage::Path notifs = (app->path / ".." / "unread.txt"); notifs.exists()) {
            storage::FileStream file(notifs.str(), storage::READ);

            if(file.size() > 0) {
                auto* notifBox = new Box(66, 0, 14, 14);
                notifBox->setRadius(7);
                notifBox->setBackgroundColor(COLOR_WARNING);
                box->addChild(notifBox);
            }

            file.close();
        }*/

        winListApps->addChild(box);

        applicationsIconsMap.insert({box, app});

        placementIndex++;
    }

    // "Overlay"
    chargingPopupBox = new Box(112, 192, 96, 96);
    chargingPopupBox->setRadius(7);
    chargingPopupBox->setBackgroundColor(TFT_BLACK);

    const auto batteryIconLightPath =
        storage::Path("system/icons/light/" + getBatteryIconFilename() + "_64px.png");
    const auto chargingIconImage = new Image(batteryIconLightPath, 16, 16, 64, 64, TFT_BLACK);
    chargingIconImage->load(TFT_BLACK);
    chargingPopupBox->addChild(chargingIconImage);

    launcherWindow->addChild(chargingPopupBox);

    // Update variables
    allocated = true;
    dirty = false;

    lastClockUpdate = os_millis();
    lastBatteryUpdate = os_millis();

    libsystem::log("end applications::launcher::draw");
}

bool applications::launcher::iconTouched()
{
    return targetApp != nullptr;
}

std::shared_ptr<AppManager::App> applications::launcher::getApp()
{
    return targetApp;
}

void applications::launcher::free()
{
    if (!allocated)
        return;

    if (launcherWindow != nullptr)
    {
        launcherWindow->free();
        launcherWindow.reset();
        launcherWindow = nullptr;
    }

    applicationsIconsMap.clear();

    clockLabel = nullptr;
    dateLabel = nullptr;
    chargingPopupBox = nullptr;

    allocated = false;
    dirty = false;
}
