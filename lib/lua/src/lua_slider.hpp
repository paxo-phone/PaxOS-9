#ifndef LUA_SLIDER_MODULE
#define LUA_SLIDER_MODULE

#include "lua_widget.hpp"

class LuaSlider : public LuaWidget
{
public:
    LuaSlider(LuaWidget *parent, int x, int y, int width, int height, int minValue, int maxValue, int defaultValue);

    void setValue(int value);
    void displayValue(bool display);
    void setMinValue(int minValue);
    void setMaxValue(int maxValue);
    int getValue();
    int getMinValue();
    int getMaxValue();
    void setBorderColor(color_t color);
    void setValueColor(color_t color);
    void setTextColor(color_t innertextColor, color_t outerTextColor);
    void setFormatPercentage(bool isPercent);

    void onChange(sol::protected_function func);
    void specificUpdate();

private:
    Slider *widget = nullptr;
    sol::protected_function onChangeFunc;
};

#endif