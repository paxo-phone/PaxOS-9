#ifndef LUA_MODULE_HPP
#define LUA_MODULE_HPP

#include "lua_box.hpp"
#include "lua_button.hpp"
#include "lua_canvas.hpp"
#include "lua_checkbox.hpp"
#include "lua_image.hpp"
#include "lua_input.hpp"
#include "lua_label.hpp"
#include "lua_list.hpp"
#include "lua_radio.hpp"
#include "lua_slider.hpp"
#include "lua_switch.hpp"
#include "lua_widget.hpp"
#include "lua_window.hpp"

class LuaFile;

struct KeyboardCallbackData
{
    sol::reference callbackRef; // Store a reference, NOT a sol::function
    lua_State* L;               // Keep the lua_State alive.

    KeyboardCallbackData(lua_State* L, sol::reference&& ref) : L(L), callbackRef(std::move(ref)) {}
};

extern std::vector<std::unique_ptr<KeyboardCallbackData>> callback_memory;

class LuaGui
{
  public:
    LuaGui(LuaFile* lua);
    ~LuaGui();
    LuaBox* box(LuaWidget* parent, int x, int y, int width, int height);
    LuaCanvas* canvas(LuaWidget* parent, int x, int y, int width, int height);
    LuaImage* image(
        LuaWidget* parent, std::string path, int x, int y, int width, int height,
        color_t background = COLOR_WHITE
    );
    LuaLabel* label(LuaWidget* parent, int x, int y, int width, int height);
    LuaInput* input(LuaWidget* parent, int x, int y);
    LuaButton* button(LuaWidget* parent, int x, int y, int width, int height);
    LuaVerticalList* verticalList(LuaWidget* parent, int x, int y, int width, int height);
    LuaHorizontalList* horizontalList(LuaWidget* parent, int x, int y, int width, int height);
    LuaCheckbox* checkbox(LuaWidget* parent, int x, int y);
    LuaSwitch* switchb(LuaWidget* parent, int x, int y);
    LuaRadio* radio(LuaWidget* parent, int x, int y);
    LuaWindow* window();
    LuaSlider* slider(
        LuaWidget* parent, int x, int y, int width, int height, int minValue, int maxValue,
        int defaultValue
    );
    std::string keyboard(
        const std::string& placeholder,
        const std::string& defaultText
    ); // deprecated
    void keyboard_async(
        const std::string& placeholder, const std::string& defaultText, sol::function callback
    );

    void del(LuaWidget* widget);

    void update();

    void setMainWindow(LuaWindow* window);
    LuaWindow* getMainWindow();
    void showInfoMessage(const std::string& msg);
    void showWarningMessage(const std::string& msg);
    void showErrorMessage(const std::string& msg);

    LuaWindow* mainWindow = nullptr;
    LuaFile* lua = nullptr;
    std::vector<LuaWidget*> widgets;
};

#endif
