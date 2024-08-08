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
        return showMessage (ERROR, msg);
    }
        
    int GuiManager::showWarningMessage(const std::string& msg ) {
        return showMessage (WARNING, msg);
    }

    int GuiManager::showInfoMessage(const std::string& msg ) {
        return showMessage (INFO, msg);
    }

        /**
         * showMessage
         * 
         * Méthode interne de création d'une popup de message de type WINDOW_TYPE
         * 
         */
        int GuiManager::showMessage(WINDOW_TYPE type, const std::string& msg ) {
            gui::elements::Window popup;

            uint16_t color_fond;
            uint16_t color_border;
            uint16_t color_text;
            std::string strTitle;

            switch(type) {
                case INFO:
                    color_fond = COLOR_WHITE;
                    color_border = COLOR_BLACK;
                    color_text = COLOR_BLACK;
                    strTitle="INFO";
                    break;
                case WARNING:
                    color_fond = COLOR_ORANGE;
                    color_border = COLOR_ORANGE;
                    color_text = COLOR_BLACK;
                    strTitle="WARNING";
                    break;
                case ERROR:
                    color_fond = COLOR_WHITE;
                    color_border = COLOR_RED;
                    color_text = COLOR_RED;
                    strTitle="ERROR";
                    break;

            }       

            gui::elements::Canvas *msgWindow = new gui::elements::Canvas(20, 120, 280, 200);
            msgWindow->fillRect(0,0,280,200,color_fond);
            msgWindow->drawRoundRect(5,5,270,190,15,color_border);
            msgWindow->drawLine(5, 30, 274, 30, color_border);

        
            gui::elements::Label *title = new gui::elements::Label(20, 7, 80, 20);
            title->setText(strTitle);
            title->setTextColor(color_text);
            msgWindow->addChild(title);

            gui::elements::Label *txt = new gui::elements::Label(20, 60, 240, 30);
            txt->setText(msg);
            txt->setVerticalAlignment(gui::elements::Label::Alignement::CENTER);
            txt->setHorizontalAlignment(gui::elements::Label::Alignement::CENTER);
            txt->setFontSize(20);
            msgWindow->addChild(txt);

            
            gui::elements::Button *btnOK = new gui::elements::Button(100, 150, 100, 30);
            btnOK->setText("OK");
            msgWindow->addChild(btnOK);
            
            popup.addChild(msgWindow);
            popup.updateAll();

            while (!hardware::getHomeButton()) {
                popup.updateAll();
                if(btnOK->isTouched()){
                    return 1;
                }
            }
            return 1;

        }