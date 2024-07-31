#ifndef GUI_MANAGER_HPP
#define  GUI_MANAGER_HPP

#include "elements/Canvas.hpp"
#include "elements/Window.hpp"
#include "elements/Label.hpp"
#include "elements/Button.hpp"
#include <hardware.hpp>

class GuiManager final {
    private:
       gui::elements::Window win;
       GuiManager();
    
    public:
        static GuiManager& getInstance();

        GuiManager(const GuiManager&) = delete;
        GuiManager& operator=(const GuiManager&) = delete;

        gui::elements::Window& getWindow() noexcept;
        int showErrorMessage(const std::string& msg );
};

#endif