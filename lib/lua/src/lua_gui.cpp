#include "lua_gui.hpp"

#include "lua_file.hpp"

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

    for (int i = 0; i < widgets.size(); i++)
    {
        if(!hasParent[i])
            delete widgets[i];
    }
}

LuaBox* LuaGui::box(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaBox* w = new LuaBox(parent, x, y, width, height);
    widgets.push_back(w);
    return w;
}

LuaImage* LuaGui::image(LuaWidget* parent, storage::Path path, int x, int y, int width, int height)
{
    // TODO: permissions relatives a l'app
    if(!this->lua->perms.acces_files)
        return nullptr;
    if(path.m_steps[0]=="/" && !this->lua->perms.acces_files_root)
        return nullptr;
    
    LuaImage* w = new LuaImage(parent, path, x, y, width, height);
    widgets.push_back(w);

    return w;
}

LuaLabel* LuaGui::label(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaLabel* w = new LuaLabel(parent, x, y, width, height);
    widgets.push_back(w);
    return w;
}

LuaInput* LuaGui::input(LuaWidget* parent, int x, int y)
{
    LuaInput* w = new LuaInput(parent, x, y);
    widgets.push_back(w);
    return w;
}

LuaButton* LuaGui::button(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaButton* w = new LuaButton(parent, x, y, width, height);
    widgets.push_back(w);
    return w;
}

LuaVerticalList* LuaGui::verticalList(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaVerticalList* w = new LuaVerticalList(parent, x, y, width, height);
    widgets.push_back(w);
    return w;
}

LuaHorizontalList* LuaGui::horizontalList(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaHorizontalList* w = new LuaHorizontalList(parent, x, y, width, height);
    widgets.push_back(w);
    return w;
}

LuaSwitch* LuaGui::switchb(LuaWidget* parent, int x, int y)
{
    LuaSwitch* w = new LuaSwitch(parent, x, y);
    widgets.push_back(w);
    return w;
}

LuaRadio* LuaGui::radio(LuaWidget* parent, int x, int y)
{
    LuaRadio* w = new LuaRadio(parent, x, y);
    widgets.push_back(w);
    return w;
}


LuaCheckbox* LuaGui::checkbox(LuaWidget* parent, int x, int y)
{
    LuaCheckbox* w = new LuaCheckbox(parent, x, y);
    widgets.push_back(w);
    return w;
}

LuaWindow* LuaGui::window()
{
    LuaWindow* win =  new LuaWindow();
    widgets.push_back(win);
    return win;
}

void LuaGui::update()
{
    if(mainWindow != nullptr)
    {
        mainWindow->update();
    }
}