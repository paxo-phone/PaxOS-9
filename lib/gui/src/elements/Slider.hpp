#ifndef SLIDER_HPP
#define SLIDER_HPP

#include "../ElementBase.hpp"

namespace gui::elements
{
    /**
     * @brief Slider widget
     **/
    class Slider final : public ElementBase
    {
    public:
        /**
         * @brief Construct a new Slider object
         *
         * @param x
         * @param y
         * @param width
         * @param height
         * @param minValue
         * @param maxValue
         * @param defaultValue
         */
        Slider(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t minValue = 0, uint16_t maxValue = 100, uint16_t defaultValue = 0);

        /**
         * @brief Destroy the Slider object
         *
         */
        ~Slider() override;

        /**
         * @brief render the slider widget
         *
         */
        void render() override;

        /**
         * @brief Set the Value object
         *
         * @param value
         */
        void setValue(uint16_t value);

        /**
         * @brief define if the value of the slide is displayed or not
         *
         * @param display
         */
        void displayValue(bool display);

        /**
         * @brief Set the Min Value object
         *
         * @param minValue
         * @return * void
         */
        void setMinValue(uint16_t minValue);

        /**
         * @brief Set the Max Value object
         *
         * @param maxValue
         */
        void setMaxValue(uint16_t maxValue);

        /**
         * @brief Get the Value of the slider
         * Value is between min and max value
         *
         * @return uint16_t
         */
        uint16_t getValue();

        /**
         * @brief Get the Min Value object
         *
         * @return uint16_t
         */
        uint16_t getMinValue();

        /**
         * @brief Get the Max Value
         *
         * @return uint16_t
         */
        uint16_t getMaxValue();

        /**
         * @brief Set the Value Color of the slider
         *
         * @param color
         */
        void setValueColor(color_t color);

        /**
         * @brief Set the Text Colors
         *
         * @param innertextColor
         * @param outerTextColor
         */
        void setTextColor(color_t innertextColor, color_t outerTextColor);

        /**
         * @brief Set the Format as Percentage
         *
         * @param isPercent boolean
         */
        void setFormatPercentage(bool isPercent);

        /**
         * @brief set the value and redraw the slider based on the position clicked
         *
         */
        void slide();

    private:
        uint16_t m_minValue;
        uint16_t m_maxValue;
        uint16_t m_Value;
        uint16_t m_positionValue;

        bool m_displayValue;
        bool m_isPercentage;

        color_t m_innerTextColor;
        color_t m_outerTextColor;
        color_t m_valueColor;
    };
} // gui::elements

#endif