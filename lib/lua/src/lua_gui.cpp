#include "lua_gui.hpp"

#include "lua_file.hpp"

LuaGui::LuaGui(LuaFile* lua)
{
    this->lua = lua;
}

LuaBox* LuaGui::box(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaBox* w = new LuaBox(parent, x, y, width, height);
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

    return w;
}

LuaLabel* LuaGui::label(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaLabel* w = new LuaLabel(parent, x, y, width, height);
    return w;
}

LuaInput* LuaGui::input(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaInput* w = new LuaInput(parent, x, y, width, height);
    return w;
}

LuaButton* LuaGui::button(LuaWidget* parent, int x, int y, int width, int height)
{
    LuaButton* w = new LuaButton(parent, x, y, width, height);
    return w;
}

LuaWindow* LuaGui::window()
{
    return new LuaWindow();
}

void LuaGui::update()
{
    if(mainWindow != nullptr)
    {
        mainWindow->update();
    }
}