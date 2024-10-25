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

    int getTextWidth(){ return widget->getTextWidth(); }
    int getTextHeight(){ return widget->getTextHeight(); }
    
    void setVerticalAlignment(Label::Alignement alignment){ widget->setVerticalAlignment(alignment); }
    void setHorizontalAlignment(Label::Alignement alignment){ widget->setHorizontalAlignment(alignment); }

    void setTextColor(color_t color) const;
    void setTextColorRGB(uint8_t r, uint8_t g, uint8_t b) const;

    Label* widget = nullptr;
};

#endif