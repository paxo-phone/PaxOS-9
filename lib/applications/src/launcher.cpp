#include <launcher.hpp>

#include <app.hpp>
#include <gsm.hpp>

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

int launcher()
{
    if(AppManager::isAnyVisibleApp())
        return -1;
    
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
    date->setText(getFormatedDate()); 
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
                date->setText(getFormatedDate());
                min = GSM::minutes;
            }
         },
        500
    );

    std::vector<gui::ElementBase*> apps;

    int placementIndex = 0;
    for (int i = 0; i < AppManager::appList.size(); i++)
    {
        if (!AppManager::appList[i].visible)
            continue;

        Box* box = new Box(60 + 119 * (placementIndex%2), 164 + 95 * int(placementIndex/2), 80, 80);
        
        Image* img = new Image(AppManager::appList[i].path / "../icon.png", 20, 6, 40, 40);
        img->load();
        box->addChild(img);

        Label* text = new Label(0, 46, 80, 34);
        text->setText(AppManager::appList[i].name);
        text->setVerticalAlignment(Label::Alignement::CENTER);
        text->setHorizontalAlignment(Label::Alignement::CENTER);
        text->setFontSize(16);
        box->addChild(text);

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