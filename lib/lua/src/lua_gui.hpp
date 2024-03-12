#ifndef LUA_MODULE_HPP
#define LUA_MODULE_HPP

#include "lua_widget.hpp"
#include "lua_box.hpp"
#include "lua_image.hpp"
#include "lua_label.hpp"
#include "lua_input.hpp"
#include "lua_button.hpp"
#include "lua_window.hpp"

class LuaFile;

class LuaGui
{
    public:
    LuaGui(LuaFile* lua);
    LuaBox* box(LuaWidget* parent, int x, int y, int width, int height);
    LuaImage* image(LuaWidget* parent, storage::Path path, int x, int y, int width, int height);
    LuaLabel* label(LuaWidget* parent, int x, int y, int width, int height);
    LuaInput* input(LuaWidget* parent, int x, int y, int width, int height);
    LuaButton* button(LuaWidget* parent, int x, int y, int width, int height);
    LuaWindow* window();

    void update();

    void setMainWindow(LuaWindow* window) {this->mainWindow = window; }
    LuaWindow* mainWindow = nullptr;
    LuaFile* lua = nullptr;
};

#endif