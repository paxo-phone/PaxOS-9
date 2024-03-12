#ifndef LUA_BUTTON_MODULE
#define LUA_BUTTON_MODULE

#include "lua_widget.hpp"

class LuaButton : public LuaWidget
{
    public:
    LuaButton(LuaWidget* parent, int x, int y, int width, int height);
    void setText(const std::string& text){ widget->setText(text); }
    std::string getText(){ return widget->getText(); }                           // I WAS HERE
    //void setFontSize(int fontSize){ widget->setFontSize(fontSize); }
    //int getTextHeight(){ return widget->getTextHeight(); }
    //void setVerticalAlignment(int alignment){ widget->setVerticalAlignment(alignment); }
    //void setHorizontalAlignment(int alignment){ widget->setHorizontalAlignment(alignment); }

    Button* widget = nullptr;
};

#endif