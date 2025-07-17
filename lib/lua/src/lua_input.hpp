#ifndef LUA_INPUT_MODULE
#define LUA_INPUT_MODULE

#include "lua_widget.hpp"

class LuaInput : public LuaWidget
{
  public:
    LuaInput(LuaWidget* parent, int x, int y);

    void setText(const std::string& text)
    {
        widget->setText(text);
    }

    void setPlaceHolder(const std::string& text)
    {
        widget->setPlaceHolder(text);
    }

    void setTitle(const std::string& text)
    {
        widget->setTitle(text);
    }

    std::string getText()
    {
        return widget->getText();
    }

    void onChange(sol::protected_function func)
    {
        onChangeFunc = func;
    }

    Input* widget = nullptr;
    sol::protected_function onChangeFunc;
};

#endif
