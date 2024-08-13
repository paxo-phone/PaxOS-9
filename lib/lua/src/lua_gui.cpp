#include "lua_gui.hpp"

#include <graphics.hpp>
#include <threads.hpp>
#include "lua_file.hpp"
#include "app.hpp"


LuaGui::LuaGui(LuaFile* lua)
{
    this->lua = lua;
}

LuaGui::~LuaGui()
{
    std::vector<bool> hasParent;

    for (int i = 0; i < widgets.size(); i++)
    {
        if(widgets[i]->widget->getParent() != nullptr)
            hasParent.push_back(true);
        else
            hasParent.push_back(false);
    }

    while (widgets.size())
    {
        delete widgets[0];
    }
}

LuaBox* LuaGui::box(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaBox* w = new LuaBox(parent, x, y, width, height);
    widgets.push_back(w);
    w->gui = this;
    return w;
}

LuaCanvas* LuaGui::canvas(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaCanvas* w = new LuaCanvas(parent, x, y, width, height, this->lua);
    widgets.push_back(w);
    w->gui = this;
    return w;
}

LuaImage* LuaGui::image(LuaWidget* parent, std::string path, int x, int y, int width, int height, color_t background)
{
    storage::Path path_(path);

    if(!this->lua->perms.acces_files)
        return nullptr;
    if(path_.m_steps[0]=="/" && !this->lua->perms.acces_files_root)
        return nullptr;
    
    if(path_.m_steps[0]!="/")
    {
        path_ = this->lua->directory / path_;
    }

    LuaImage* w = new LuaImage(parent, path_, x, y, width, height, background);
    widgets.push_back(w);
    w->gui = this;

    return w;
}

LuaLabel* LuaGui::label(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaLabel* w = new LuaLabel(parent, x, y, width, height);
    widgets.push_back(w);
    w->gui = this;
    return w;
}

LuaInput* LuaGui::input(LuaWidget* parent, int x, int y)
{
    LuaInput* w = new LuaInput(parent, x, y);
    widgets.push_back(w);
    w->gui = this;
    return w;
}

LuaButton* LuaGui::button(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaButton* w = new LuaButton(parent, x, y, width, height);
    widgets.push_back(w);
    w->gui = this;
    return w;
}

LuaVerticalList* LuaGui::verticalList(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaVerticalList* w = new LuaVerticalList(parent, x, y, width, height);
    widgets.push_back(w);
    w->gui = this;
    return w;
}

LuaHorizontalList* LuaGui::horizontalList(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaHorizontalList* w = new LuaHorizontalList(parent, x, y, width, height);
    widgets.push_back(w);
    w->gui = this;
    return w;
}

LuaSwitch* LuaGui::switchb(LuaWidget* parent, int x, int y)
{
    LuaSwitch* w = new LuaSwitch(parent, x, y, this);
    widgets.push_back(w);
    w->gui = this;
    return w;
}

LuaRadio* LuaGui::radio(LuaWidget* parent, int x, int y)
{
    LuaRadio* w = new LuaRadio(parent, x, y);
    widgets.push_back(w);
    w->gui = this;
    return w;
}


LuaCheckbox* LuaGui::checkbox(LuaWidget* parent, int x, int y)
{
    LuaCheckbox* w = new LuaCheckbox(parent, x, y);
    widgets.push_back(w);
    w->gui = this;
    return w;
}

LuaWindow* LuaGui::window()
{
    LuaWindow* win =  new LuaWindow();
    widgets.push_back(win);
    win->gui = this;
    return win;
}

void LuaGui::del(LuaWidget* widget)
{
    delete widget;
    if(mainWindow == widget)
        mainWindow = nullptr;
    widget = nullptr;
}

void LuaGui::update()
{
    if(mainWindow != nullptr)
    {
        mainWindow->update();
    }
}

std::string LuaGui::keyboard(const std::string& placeholder, const std::string& defaultText)
{
    graphics::setScreenOrientation(graphics::LANDSCAPE);

    auto key = new Keyboard(defaultText);
    key->setPlaceholder(placeholder);

    while (!hardware::getHomeButton() && !key->quitting())
    {
        eventHandlerApp.update();
        key->updateAll();
    }

    graphics::setScreenOrientation(graphics::PORTRAIT);

    std::string o = key->getText();

    delete key;
    return o;
}

void LuaGui::setMainWindow(LuaWindow* window) {
    this->mainWindow = window; 
    AppManager::askGui(this->lua); 
}

void LuaGui::showErrorMessage(const std::string& msg ){

        GuiManager &guiManager = GuiManager::getInstance();
        guiManager.showErrorMessage(msg);

}

void LuaGui::showWarningMessage(const std::string& msg ){

        GuiManager &guiManager = GuiManager::getInstance();
        guiManager.showWarningMessage(msg);

}
void LuaGui::showInfoMessage(const std::string& msg ){

        GuiManager &guiManager = GuiManager::getInstance();
        guiManager.showInfoMessage(msg);

}