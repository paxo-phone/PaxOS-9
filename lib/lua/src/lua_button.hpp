#ifndef LUA_BUTTON_MODULE
#define LUA_BUTTON_MODULE

#include "lua_widget.hpp"

#include <path.hpp>

class LuaButton : public LuaWidget
{
  public:
    LuaButton(LuaWidget* parent, int x, int y, int width, int height);

    void setText(const std::string& text)
    {
        widget->setText(text);
    }

    std::string getText()
    {
        return widget->getText();
    }

    void setIcon(const std::string& path)
    {
        widget->setIcon(storage::Path(path));
    }

    void setTheme(bool theme)
    {
        widget->setTheme(theme);
    }

    void format()
    {
        widget->format();
    }

    Button* widget = nullptr;
};

#endif
