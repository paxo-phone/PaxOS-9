#ifndef GUI_MANAGER_HPP
#define  GUI_MANAGER_HPP

#include "elements/Canvas.hpp"
#include "elements/Window.hpp"
#include "elements/Label.hpp"
#include "elements/Button.hpp"
#include <hardware.hpp>

enum WINDOW_TYPE {
    INFO, 
    WARNING,
    ERROR
};

class GuiManager final {
    private:
       gui::elements::Window win;
       GuiManager();
       int showMessage(WINDOW_TYPE type, const std::string& msg );

    
    public:
        static GuiManager& getInstance();

        GuiManager(const GuiManager&) = delete;
        GuiManager& operator=(const GuiManager&) = delete;

        gui::elements::Window& getWindow() noexcept;
        int showInfoMessage(const std::string& msg );
        int showWarningMessage(const std::string& msg );
        int showErrorMessage(const std::string& msg );
};

#endif