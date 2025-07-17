/**
 * @file Slider.cpp
 * @author PYM
 * @brief Slide widget
 * @version 0.1
 *
 */
#include "Slider.hpp"

#include <Surface.hpp>
#include <cmath>
#include <cstdio>
#include <graphics.hpp>
#include <iostream>

namespace gui::elements
{
    Slider::Slider(
        uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t minValue,
        uint16_t maxValue, uint16_t defaultValue
    )
    {
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
        m_minValue = minValue;
        m_maxValue = maxValue;
        m_borderSize = 2;
        m_borderRadius = m_height / 2;
        if (defaultValue >= minValue && defaultValue <= maxValue)
            m_Value = defaultValue;
        else
            m_Value = minValue;
        m_borderColor = graphics::constPackRGB565(217, 217, 217);
        ;
        m_backgroundColor = m_borderColor;
        m_valueColor = COLOR_BLACK;
        m_positionValue = 0;
        m_displayValue = false;
        m_innerTextColor = graphics::constPackRGB565(250, 250, 250); // COLOR_LIGHT_GREY;
        m_outerTextColor = COLOR_BLACK;
        m_isPercentage = false;
        m_hasEvents = true;
    }

    Slider::~Slider() = default;

    void Slider::setValueColor(color_t color)
    {
        m_valueColor = color;
    }

    uint16_t Slider::getValue()
    {
        return m_Value;
    }

    uint16_t Slider::getMinValue()
    {
        return m_maxValue;
    }

    uint16_t Slider::getMaxValue()
    {
        return m_maxValue;
    }

    void Slider::setValue(uint16_t value)
    {
        if (value > m_maxValue)
            m_Value = m_maxValue;
        else if (value < m_minValue)
            m_Value = m_minValue;
        else
            m_Value = value;
        localGraphicalUpdate();
    }

    void Slider::setMinValue(uint16_t minValue)
    {
        m_minValue = minValue;
    }

    void Slider::setMaxValue(uint16_t maxValue)
    {
        m_maxValue = maxValue;
    }

    void Slider::displayValue(bool display)
    {
        m_displayValue = display;
    }

    void Slider::setTextColor(color_t innertextColor, color_t outerTextColor)
    {
        m_innerTextColor = innertextColor;
        m_outerTextColor = outerTextColor;
    }

    void Slider::setFormatPercentage(bool isPercent)
    {
        m_isPercentage = isPercent;
    }

    void Slider::slide()
    {
        // m_positionValue = (touchX - getAbsoluteX());
        // m_Value = (m_positionValue * (m_maxValue - m_minValue)) / m_width + m_minValue;
        float value = (touchX - getAbsoluteX()) * (m_maxValue - m_minValue) / m_width + m_minValue;
        value = value < m_minValue ? m_minValue : value > m_maxValue ? m_maxValue : value;

        if (m_Value != value)
        {
            setValue(value);
            m_hasChanged = true;
        }
    }

    void Slider::widgetUpdate()
    {
        if (widgetPressed == this &&
            (globalPressedState == PressedState::PRESSED ||
             globalPressedState == PressedState::SCROLLX) &&
            touchX >= 0 && touchY >= 0)
        {
            slide();
        }
    }

    void Slider::render()
    {

        this->getAndSetSurface()->clear(
            m_parent == nullptr ? COLOR_WHITE : m_parent->getBackgroundColor()
        );

        // render the of the widget
        int valueSize = ((m_Value - m_minValue) * m_width) / (m_maxValue - m_minValue);
        m_positionValue = valueSize + m_x;

        this->getAndSetSurface()->fillRoundRectWithBorder(
            0,
            0,
            m_width,
            m_height,
            m_borderRadius,
            m_borderSize,
            m_backgroundColor,
            m_borderColor
        );

        // render the "0 -> value" part
        this->getAndSetSurface()->fillRoundRectWithBorder(
            0,
            0,
            valueSize,
            m_height,
            m_borderRadius,
            m_borderSize,
            m_valueColor,
            m_borderColor
        );

        if (m_displayValue)
        {
            int x = 0;
            std::string str = std::to_string(m_Value);
            if (m_isPercentage)
                str = str + "%";
            const uint16_t textWidth = this->getAndSetSurface()->m_sprite.textWidth(str.c_str());
            const uint16_t textHeight = this->getAndSetSurface()->getTextHeight();
            if (textWidth <= valueSize - 10)
            {
                // this->getAndSetSurface()->setTextColor(m_innerTextColor);
                this->getAndSetSurface()->drawText(
                    str,
                    valueSize - textWidth - 5,
                    floor((m_height - textHeight) / 2),
                    m_innerTextColor
                );
            }
            else
            {
                //                this->getAndSetSurface()->setTextColor(COLOR_GREEN);
                this->getAndSetSurface()->drawText(
                    str,
                    valueSize + 5,
                    floor((m_height - textHeight) / 2),
                    m_outerTextColor
                );
            }
        }
    }

} // namespace gui::elements
