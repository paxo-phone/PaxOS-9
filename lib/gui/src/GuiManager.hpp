#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP

#include "elements/Window.hpp"

enum WINDOW_TYPE
{
    INFO,
    WARNING,
    ERROR
};

class GuiManager final
{
    gui::elements::Window win;
    gui::elements::Window* lastUpdatedWindow_;
    std::vector<std::function<void(gui::elements::Window*)>> windowChangedCallbacks_;

    GuiManager();
    int showMessage(WINDOW_TYPE type, const std::string& msg);

  public:
    static GuiManager& getInstance();

    GuiManager(const GuiManager&) = delete;
    GuiManager& operator=(const GuiManager&) = delete;

    gui::elements::Window& getWindow() noexcept;
    int showInfoMessage(const std::string& msg);
    int showWarningMessage(const std::string& msg);
    int showErrorMessage(const std::string& msg);

    void onWindowUpdate(gui::elements::Window* window);

    void addWindowChangedCallback(const std::function<void(gui::elements::Window*)>& callback);
};

#endif
