#ifndef LUA_MODULE_HPP
#define LUA_MODULE_HPP

#include "lua_widget.hpp"
#include "lua_box.hpp"
#include "lua_image.hpp"
#include "lua_label.hpp"
#include "lua_input.hpp"
#include "lua_button.hpp"
#include "lua_window.hpp"
#include "lua_list.hpp"
#include "lua_switch.hpp"
#include "lua_radio.hpp"
#include "lua_checkbox.hpp"

class LuaFile;

class LuaGui
{
    public:
    LuaGui(LuaFile* lua);
    ~LuaGui();
    LuaBox* box(LuaWidget* parent, int x, int y, int width, int height);
    LuaImage* image(LuaWidget* parent, storage::Path path, int x, int y, int width, int height);
    LuaLabel* label(LuaWidget* parent, int x, int y, int width, int height);
    LuaInput* input(LuaWidget* parent, int x, int y);
    LuaButton* button(LuaWidget* parent, int x, int y, int width, int height);
    LuaVerticalList* verticalList(LuaWidget* parent, int x, int y, int width, int height);
    LuaHorizontalList* horizontalList(LuaWidget* parent, int x, int y, int width, int height);
    LuaCheckbox* checkbox(LuaWidget* parent, int x, int y);
    LuaSwitch* switchb(LuaWidget* parent, int x, int y);
    LuaRadio* radio(LuaWidget* parent, int x, int y);
    LuaWindow* window();

    void update();

    void setMainWindow(LuaWindow* window) {this->mainWindow = window; }
    LuaWindow* mainWindow = nullptr;
    LuaFile* lua = nullptr;
    std::vector<LuaWidget*> widgets;
};

#endif