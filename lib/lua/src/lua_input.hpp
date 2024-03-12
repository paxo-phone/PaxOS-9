#ifndef LUA_INPUT_MODULE
#define LUA_INPUT_MODULE

#include "lua_widget.hpp"


class LuaInput : public LuaWidget
{
    public:
    LuaInput(LuaWidget* parent, int x, int y, int width, int height);
    void setText(const std::string& text){ widget->setText(text); }
    std::string getText(){ return widget->getText(); }
    //void setFontSize(int fontSize){ widget->setFontSize(fontSize); }
    //int getTextHeight(){ return widget->getTextHeight(); }
    void onChange(sol::function func){ onChangeFunc = func;}
    //void setVerticalAlignment(int alignment){ widget->setVerticalAlignment(alignment); }
    //void setHorizontalAlignment(int alignment){ widget->setHorizontalAlignment(alignment); }

    Input* widget = nullptr;
    sol::function onChangeFunc;
};

#endif