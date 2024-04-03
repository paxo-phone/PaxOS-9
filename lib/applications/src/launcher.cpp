#include <launcher.hpp>

#include <app.hpp>

int launcher()
{
    gui::elements::Window win;

    Label *hour = new Label(86, 42, 148, 41);
    hour->setText("10:30");    // hour
    hour->setVerticalAlignment(Label::Alignement::CENTER);
    hour->setHorizontalAlignment(Label::Alignement::CENTER);
    hour->setFontSize(36);
    win.addChild(hour);
    
    Label *date = new Label(55, 89, 210, 18);
    date->setText("Lundi 25 Mars");
    date->setVerticalAlignment(Label::Alignement::CENTER);
    date->setHorizontalAlignment(Label::Alignement::CENTER);
    date->setFontSize(16);
    win.addChild(date);

    std::vector<gui::ElementBase*> apps;

    for (int i = 0; i < app::appList.size(); i++)
    {
        Box* box = new Box(60 + 119 * (i%2), 164 + 95 * int(i/2), 80, 80);
        
        std::cout << (app::appList[i].path / "../icon.png").str() << std::endl;
        Image* img = new Image(app::appList[i].path / "../icon.png", 20, 6, 40, 40);
        img->load();
        box->addChild(img);

        Label* text = new Label(0, 46, 80, 34);
        text->setText(app::appList[i].name);
        text->setVerticalAlignment(Label::Alignement::CENTER);
        text->setHorizontalAlignment(Label::Alignement::CENTER);
        text->setFontSize(16);
        box->addChild(text);

        win.addChild(box);

        apps.push_back(box);
    }

    while (!hardware::getHomeButton())
    {
        for (int i = 0; i < apps.size(); i++)
        {
            if(apps[i]->isTouched())
            {
                return i;
            }
        }

        if (app::request)
        {
            app::request = false;
            app::runApp(app::requestingApp);
        }

        win.updateAll();
    }

    return -1;
}