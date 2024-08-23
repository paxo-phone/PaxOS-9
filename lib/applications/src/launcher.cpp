#include <launcher.hpp>

#include <app.hpp>
#include <gsm.hpp>
#include <libsystem.hpp>
#include <memory>

std::string getFormatedHour()
{
    uint16_t day_ = GSM::days;
    uint16_t day = GSM::days;
    uint16_t month = GSM::months;
    uint16_t year = GSM::years;

    std::string dayName = daysOfWeek[(day+=month<3?year--:year-2,23*month/9+day+4+year/4-year/100+year/400)%7];
    std::string monthName = daysOfMonth[month==0?1:(month-1)];

    return dayName + " " + std::to_string(GSM::days) + " " + monthName;
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
    Box* chargingPopupBox = nullptr;

    uint64_t lastClockUpdate = 0;
    uint64_t lastBatteryUpdate = 0;

    uint64_t chargingStartTime = 0;
}

void applications::launcher::init() {
    launcherWindow = std::make_shared<Window>();
}

void applications::launcher::update() {
    if (dirty) {
        // If dirty, free to force redraw it
        free();
    }

    if (!allocated) {
        // If launcher has been freed, redraw it
        draw();
    }

    // Update dynamic elements
    // TODO : Refactor this
    if (millis() > lastClockUpdate + 1000) {
        // What ???
        static int min;

        if(min != GSM::minutes) {
            clockLabel->setText(std::to_string(GSM::hours) + ":" + (GSM::minutes<=9 ? "0" : "") + std::to_string(GSM::minutes));
            dateLabel->setText(getFormatedHour());

            min = GSM::minutes;
        }

        lastClockUpdate = millis();
    }
    if (millis() > lastBatteryUpdate + 10000) {
        batteryLabel->setText(std::to_string(static_cast<int>(GSM::getBatteryLevel() * 100)) + "%");

        lastBatteryUpdate = millis();
    }

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

    // Update, draw AND update touch events
    if (launcherWindow != nullptr) {
        launcherWindow->updateAll();
    }

    // Check touch events
    targetApp = nullptr;

    for (const auto& [icon, app] : applicationsIconsMap) {
        if (icon->isTouched()) {
            targetApp = app;
        }
    }
}

void applications::launcher::draw() {
    libsystem::log("applications::launcher::draw");

    if (launcherWindow == nullptr) {
        launcherWindow = std::make_shared<Window>();
    }

    StandbyMode::triggerPower();

    // Clock
    clockLabel = new Label(86, 42, 148, 41);
    clockLabel->setText(std::to_string(GSM::hours) + ":" + (GSM::minutes<=9 ? "0" : "") + std::to_string(GSM::minutes));    // hour
    clockLabel->setVerticalAlignment(Label::Alignement::CENTER);
    clockLabel->setHorizontalAlignment(Label::Alignement::CENTER);
    clockLabel->setFontSize(36);
    launcherWindow->addChild(clockLabel);

    // Date
    dateLabel = new Label(55, 89, 210, 18);
    dateLabel->setText(getFormatedHour());
    dateLabel->setVerticalAlignment(Label::Alignement::CENTER);
    dateLabel->setHorizontalAlignment(Label::Alignement::CENTER);
    dateLabel->setFontSize(16);
    launcherWindow->addChild(dateLabel);

    // Battery
    batteryLabel = new Label(269, 10, 40, 18);
    batteryLabel->setText(std::to_string(static_cast<int>(GSM::getBatteryLevel() * 100)) + "%");
    batteryLabel->setVerticalAlignment(Label::Alignement::CENTER);
    batteryLabel->setHorizontalAlignment(Label::Alignement::CENTER);
    batteryLabel->setFontSize(18);
    launcherWindow->addChild(batteryLabel);

    // Network
    if(GSM::getNetworkStatus() == 99) {
        auto* networkLabel = new Label(10, 10, 100, 18);
        networkLabel->setText("No network");
        networkLabel->setVerticalAlignment(Label::Alignement::CENTER);
        networkLabel->setHorizontalAlignment(Label::Alignement::CENTER);
        networkLabel->setFontSize(18);
        launcherWindow->addChild(networkLabel);
    }

    std::vector<gui::ElementBase*> apps;

    int placementIndex = 0;

    for (const auto& app : AppManager::appList) {
        if (!app->visible) {
            // If an app is not visible (AKA. Background app)
            // Skip it

            continue;
        }

        auto* box = new Box(60 + 119 * (placementIndex%2), 164 + 95 * (placementIndex / 2), 80, 80);

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

        launcherWindow->addChild(box);

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

    const auto path = storage::Path("system/icons/battery_charging_full_64px.png");

    libsystem::log("Path: " + path.str());

    const auto chargingIconImage = new Image(
        path,
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

    // Is this the inverse of "triggerPower()" ?
    StandbyMode::restorePower();
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

int launcher()
{
    if(AppManager::isAnyVisibleApp()) {
        return -1;
    }

    
    StandbyMode::triggerPower();
    gui::elements::Window win;

    Label *hour = new Label(86, 42, 148, 41);
    hour->setText(std::to_string(GSM::hours) + ":" + (GSM::minutes<=9 ? "0" : "") + std::to_string(GSM::minutes));    // hour
    hour->setVerticalAlignment(Label::Alignement::CENTER);
    hour->setHorizontalAlignment(Label::Alignement::CENTER);
    hour->setFontSize(36);
    win.addChild(hour);
    
    Label *date = new Label(55, 89, 210, 18);
    Date data = {GSM::days, GSM::months, GSM::years};
    date->setText(getFormatedHour()); 
    date->setVerticalAlignment(Label::Alignement::CENTER);
    date->setHorizontalAlignment(Label::Alignement::CENTER);
    date->setFontSize(16);
    win.addChild(date);

    Label *batt = new Label(269, 10, 40, 18);
    batt->setText(std::to_string(GSM::getBatteryLevel()));    // hour
    batt->setVerticalAlignment(Label::Alignement::CENTER);
    batt->setHorizontalAlignment(Label::Alignement::CENTER);
    batt->setFontSize(18);
    win.addChild(batt);

    if(GSM::getNetworkStatus() == 99)
    {
        Label *batt = new Label(10, 10, 100, 18);
        batt->setText("pas de réseau");    // hour
        batt->setVerticalAlignment(Label::Alignement::CENTER);
        batt->setHorizontalAlignment(Label::Alignement::CENTER);
        batt->setFontSize(18);
        win.addChild(batt);
    }

    uint32_t evid = eventHandlerApp.setInterval(
        [&hour, &date]() { 
            static int min;
            if(min!=GSM::minutes)
            {
                hour->setText(std::to_string(GSM::hours) + ":" + (GSM::minutes<=9 ? "0" : "") + std::to_string(GSM::minutes));
                date->setText(getFormatedHour());
                min = GSM::minutes;
            }
         },
        500
    );

    std::vector<gui::ElementBase*> apps;

    int placementIndex = 0;
    for (int i = 0; i < AppManager::appList.size(); i++)
    {
        if (!AppManager::appList[i]->visible)
            continue;

        Box* box = new Box(60 + 119 * (placementIndex%2), 164 + 95 * int(placementIndex/2), 80, 80);
        
        Image* img = new Image(AppManager::appList[i]->path / "../icon.png", 20, 6, 40, 40);
        img->load();
        box->addChild(img);

        Label* text = new Label(0, 46, 80, 34);
        text->setText(AppManager::appList[i]->name);
        text->setVerticalAlignment(Label::Alignement::CENTER);
        text->setHorizontalAlignment(Label::Alignement::CENTER);
        text->setFontSize(16);
        box->addChild(text);

        storage::Path notifs = (AppManager::appList[i]->path / ".." / "unread.txt");
        if(notifs.exists())
        {
            storage::FileStream file(notifs.str(), storage::READ);
            
            if(file.size() > 0)
            {
                Box* notifBox = new Box(66, 0, 14, 14);
                notifBox->setRadius(7);
                notifBox->setBackgroundColor(COLOR_WARNING);
                box->addChild(notifBox);
            }

            file.close();
        }

        win.addChild(box);

        apps.push_back(box);
        
        placementIndex++;
    }

    while (!hardware::getHomeButton() && AppManager::isAnyVisibleApp() == false)
    {
        for (int i = 0; i < apps.size(); i++)
        {
            if(apps[i]->isTouched())
            {
                eventHandlerApp.removeInterval(evid);
                return i;
            }
        }

        eventHandlerApp.update();
        win.updateAll();

        AppManager::loop();
    }

    eventHandlerApp.removeInterval(evid);
    return -1;
}
