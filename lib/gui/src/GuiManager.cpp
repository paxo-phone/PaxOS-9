#include "GuiManager.hpp"

    
    GuiManager::GuiManager(){

    }

    GuiManager& GuiManager::getInstance() {
        static GuiManager instance; // Instanciation unique
        return instance;
    }
    
    gui::elements::Window& GuiManager::getWindow() noexcept{
        return win;
    }

    int GuiManager::showErrorMessage(const std::string& msg ) {
            gui::elements::Window popup;

            gui::elements::Canvas *errorWindow = new gui::elements::Canvas(20, 120, 280, 200);
            errorWindow->fillRect(0,0,280,200,COLOR_WHITE);
            errorWindow->drawRoundRect(5,5,270,190,15,COLOR_RED);
            errorWindow->drawLine(5, 30, 274, 30, COLOR_RED);

        
            gui::elements::Label *title = new gui::elements::Label(20, 7, 80, 20);
            title->setText("ERROR");
            title->setTextColor(COLOR_RED);
            errorWindow->addChild(title);

            gui::elements::Label *txt = new gui::elements::Label(20, 60, 240, 30);
            txt->setText(msg);
            txt->setVerticalAlignment(gui::elements::Label::Alignement::CENTER);
            txt->setHorizontalAlignment(gui::elements::Label::Alignement::CENTER);
            txt->setFontSize(20);
            errorWindow->addChild(txt);

            
            gui::elements::Button *btnOK = new gui::elements::Button(100, 150, 100, 30);
            btnOK->setText("OK");
            errorWindow->addChild(btnOK);
            
            popup.addChild(errorWindow);
            popup.updateAll();

            while (!hardware::getHomeButton()) {
                popup.updateAll();
                if(btnOK->isTouched()){
                    return 1;
                }
            }
            return 1;

        }
        
