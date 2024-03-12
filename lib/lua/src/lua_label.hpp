#ifndef LUA_LABEL_MODULE
#define LUA_LABEL_MODULE

#include "lua_widget.hpp"

class LuaLabel : public LuaWidget
{
    public:
    LuaLabel(LuaWidget* parent, int x, int y, int width, int height);
    void setText(const std::string& text){ widget->setText(text); }
    std::string getText(){ return widget->getText(); }
    void setFontSize(int fontSize){ widget->setFontSize(fontSize); }
    int getTextHeight(){ return widget->getTextHeight(); }
    //void setBold(bool bold){ widget->setBold(bold); }
    //void setItalic(bool italic){ widget->setItalic(italic); }
    void setVerticalAlignment(Label::Alignement alignment){ widget->setVerticalAlignment(alignment); }
    void setHorizontalAlignment(Label::Alignement alignment){ widget->setHorizontalAlignment(alignment); }

    Label* widget = nullptr;
};

#endif