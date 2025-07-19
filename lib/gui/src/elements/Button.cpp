#include "Button.hpp"

#include <color.hpp>
#include <iostream>

namespace gui::elements
{
    Button::Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        this->m_x = x;
        this->m_y = y;
        this->m_width = width;
        this->m_height = height;

        this->m_label = nullptr;
        this->m_image = nullptr;

        this->m_borderColor = COLOR_DARK;
        this->m_borderSize = 2;
        this->m_backgroundColor = COLOR_WHITE;
        this->m_borderRadius = 17;
        this->m_backgroundColorSave = m_borderSize;

        m_theme = BUTTON_WHITE;

        this->m_label = nullptr;
        this->m_image = nullptr;
    }

    Button::~Button() = default;

    void Button::render()
    {
        m_surface->clear(m_parent == nullptr ? COLOR_WHITE : m_parent->getBackgroundColor());
        m_surface->fillRoundRectWithBorder(
            0,
            0,
            this->m_width,
            this->m_height,
            this->m_borderRadius,
            this->m_borderSize,
            this->m_backgroundColor,
            this->m_borderColor
        );
    }

    void Button::onClick() //
    {
        m_backgroundColorSave = this->m_borderSize;
        this->m_borderSize += 2;

        // m_surface->applyFilter(graphics::Surface::Filter::DARKEN, 100);

        /*uint8_t r, g, b;

        graphics::unpackRGB565(this->m_backgroundColor, &r, &g, &b);

        r/= 3;
        g/= 3;
        b/= 3;

        this->m_backgroundColor = graphics::packRGB565(r, g, b);*/

        localGraphicalUpdate();

        // m_isDrawn = false;
    }

    void Button::onNotClicked()
    {
        // this->m_backgroundColor = this->m_backgroundColorSave;
        m_borderSize = m_backgroundColorSave;
        localGraphicalUpdate();
    }

    void Button::format()
    {
        uint16_t space = (m_image != nullptr && m_label != nullptr) ? 10 : 0;
        uint16_t w = space;

        if (m_image != nullptr)
            w += m_image->getWidth();
        if (m_label != nullptr)
            w += m_label->getTextWidth();

        if (m_image != nullptr)
            m_image->setX(getWidth() / 2 - w / 2);

        if (m_label != nullptr)
        {
            m_label->setX(
                space + ((m_image != nullptr) ? (m_image->getX() + m_image->getWidth())
                                              : getWidth() / 2 - w / 2)
            );
            m_label->setY(10);
            m_label->setWidth(m_label->getTextWidth());
            m_label->setHeight(18);
            m_label->setFontSize(LABEL_SMALL);

            if (m_theme)
            {
                m_label->setTextColor(COLOR_DARK);
                m_label->setBackgroundColor(COLOR_WHITE);
            }
            else
            {
                m_label->setTextColor(COLOR_WHITE - 1);
                m_label->setBackgroundColor(COLOR_DARK);
            }
        }

        if (m_theme)
            this->setBackgroundColor(COLOR_WHITE);
        else
            this->setBackgroundColor(COLOR_DARK);
    }

    void Button::setText(std::string text)
    {
        if (m_label == nullptr)
        {
            m_label = new Label(0, 0, 0, 0);
            addChild(m_label);
        }
        this->m_label->setText(text);
        format();
    }

    void Button::setIcon(storage::Path path)
    {
        if (m_image == nullptr)
        {
            m_image = new Image(path, 0, 10, 20, 20);
            m_image->load();
            addChild(m_image);
            format();
        }
    }

    std::string Button::getText()
    {
        if (m_label == nullptr)
            return "";
        else
            return m_label->getText();
    }

    void Button::setTheme(bool value)
    {
        this->m_theme = value;
    }
} // namespace gui::elements
