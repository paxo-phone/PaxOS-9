#include "lua_slider.hpp"

LuaSlider::LuaSlider(
    LuaWidget* parent, int x, int y, int width, int height, int minValue, int maxValue,
    int defaultValue
)
{
    widget = new Slider(x, y, width, height, minValue, maxValue, defaultValue);
    init(widget, parent);
}

void LuaSlider::setValue(int value)
{
    widget->setValue(value);
}

void LuaSlider::displayValue(bool display)
{
    widget->displayValue(display);
}

void LuaSlider::setMinValue(int minValue)
{
    widget->setMinValue(minValue);
}

void LuaSlider::setMaxValue(int maxValue)
{
    widget->setMaxValue(maxValue);
}

int LuaSlider::getValue()
{
    return widget->getValue();
}

int LuaSlider::getMinValue()
{
    return widget->getMinValue();
}

int LuaSlider::getMaxValue()
{
    return widget->getMaxValue();
}

void LuaSlider::setBorderColor(color_t color)
{
    widget->setBorderColor(color);
}

void LuaSlider::setValueColor(color_t color)
{
    widget->setValueColor(color);
}

void LuaSlider::setTextColor(color_t innertextColor, color_t outerTextColor)
{
    widget->setTextColor(innertextColor, outerTextColor);
}

void LuaSlider::setFormatPercentage(bool isPercent)
{
    widget->setFormatPercentage(isPercent);
}

void LuaSlider::onChange(sol::protected_function func)
{
    onChangeFunc = func;
}

void LuaSlider::specificUpdate()
{
    if (onChangeFunc && widget->hasChanged())
        onChangeFunc();
}
