#include "lua_gui.hpp"

#include "app.hpp"
#include "libsystem.hpp"
#include "lua_file.hpp"

#include <graphics.hpp>
#include <threads.hpp>

std::vector<std::unique_ptr<KeyboardCallbackData>> callback_memory;

LuaGui::LuaGui(LuaFile* lua)
{
    this->lua = lua;
}

LuaGui::~LuaGui()
{
    std::vector<bool> hasParent;

    for (int i = 0; i < widgets.size(); i++)
        if (widgets[i]->widget->getParent() != nullptr)
            hasParent.push_back(true);
        else
            hasParent.push_back(false);

    while (widgets.size()) delete widgets[0];
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

LuaImage* LuaGui::image(
    LuaWidget* parent, std::string path, int x, int y, int width, int height, color_t background
)
{
    storage::Path path_(path);

    if (!this->lua->perms.acces_files)
        return nullptr;
    if (path_.m_steps[0] == "/" && !this->lua->perms.acces_files_root)
        return nullptr;

    if (path_.m_steps[0] != "/")
        path_ = this->lua->directory / path_;

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

LuaSlider* LuaGui::slider(
    LuaWidget* parent, int x, int y, int width, int height, int minValue, int maxValue,
    int defaultValue
)
{
    LuaSlider* slider =
        new LuaSlider(parent, x, y, width, height, minValue, maxValue, defaultValue);
    widgets.push_back(slider);
    slider->gui = this;
    return slider;
}

LuaWindow* LuaGui::window()
{
    LuaWindow* win = new LuaWindow();
    widgets.push_back(win);
    win->gui = this;
    return win;
}

void LuaGui::del(LuaWidget* widget)
{
    // prevent a widget to remove itself during its execution
    if (lua->lua_time.running)
    {
        delete widget;
        if (mainWindow == widget)
            mainWindow = nullptr;
        widget = nullptr;
    }
    else // auto enable async if not already done by lua
    {
        lua->eventHandler.setTimeout(
            new Callback<>(std::bind(
                std::function<void(LuaWidget*, LuaWidget*)>(
                    [](LuaWidget* widget, LuaWidget* mainWindow)
                    {
                        delete widget;
                        if (mainWindow == widget)
                            mainWindow = nullptr;
                        widget = nullptr;
                    }
                ),
                widget,
                mainWindow
            )),
            0
        );
    }
}

void LuaGui::update()
{
    if (mainWindow != nullptr)
        mainWindow->update();
}

std::string LuaGui::keyboard(const std::string& placeholder, const std::string& defaultText)
{
    libsystem::log("[WARNING]: Keyboard is deprecated. Use keyboard_async instead.");
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

void LuaGui::keyboard_async(
    const std::string& placeholder, const std::string& defaultText, sol::function callback
)
{
    printf("Calling keyboard_async\n");
    // Store a REFERENCE to the callback, NOT a copy of the sol::function.
    callback_memory.push_back(
        std::make_unique<KeyboardCallbackData>(
            lua->lua.lua_state(),
            sol::reference(lua->lua.lua_state(), callback)
        )
    );

    AppManager::Keyboard_manager::open(
        this->lua->app,
        placeholder,
        defaultText,
        [this, index = callback_memory.size() - 1](std::string t)
        {
            // Get the callback reference from storage
            KeyboardCallbackData* data = callback_memory[index].get();
            if (data && data->callbackRef.valid())
            {
                sol::protected_function callbackFunc(data->callbackRef);
                sol::protected_function_result result = callbackFunc(t);

                if (!result.valid())
                {
                    sol::error err = result;
                    std::cerr << "Callback error: " << err.what() << std::endl;

                    // Show error (same as your pushError function)
                    std::string error_message =
                        "The callback for keyboard_async encountered an error: ";
                    error_message += err.what();
                    showErrorMessage(error_message);
                }

                // Optionally Remove, see notes below
                // callback_memory[index].reset();
                // callback_memory.erase(callback_memory.begin() + index);
            }
        }
    );
    // printf("end keyboard_async");
}

void LuaGui::setMainWindow(LuaWindow* window)
{
    this->mainWindow = window;
}

LuaWindow* LuaGui::getMainWindow()
{
    return this->mainWindow;
}

void LuaGui::showErrorMessage(const std::string& msg)
{

    GuiManager& guiManager = GuiManager::getInstance();
    guiManager.showErrorMessage(msg);
}

void LuaGui::showWarningMessage(const std::string& msg)
{

    GuiManager& guiManager = GuiManager::getInstance();
    guiManager.showWarningMessage(msg);
}

void LuaGui::showInfoMessage(const std::string& msg)
{

    GuiManager& guiManager = GuiManager::getInstance();
    guiManager.showInfoMessage(msg);
}
