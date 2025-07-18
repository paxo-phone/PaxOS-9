#include "Button.hpp"

#include <color.hpp>
#include <iostream>

namespace gui::elements
{
    Button::Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        this->x_ = x;
        this->y_ = y;
        this->width_ = width;
        this->height_ = height;

        this->m_label = nullptr;
        this->m_image = nullptr;

        this->borderColor_ = COLOR_DARK;
        this->borderSize_ = 2;
        this->backgroundColor_ = COLOR_WHITE;
        this->borderRadius_ = 17;
        this->m_backgroundColorSave = borderSize_;

        m_theme = BUTTON_WHITE;

        this->m_label = nullptr;
        this->m_image = nullptr;
    }

    Button::~Button() = default;

    void Button::render()
    {
        surface_->clear(parent_ == nullptr ? COLOR_WHITE : parent_->getBackgroundColor());
        surface_->fillRoundRectWithBorder(
            0,
            0,
            this->width_,
            this->height_,
            this->borderRadius_,
            this->borderSize_,
            this->backgroundColor_,
            this->borderColor_
        );
    }

    void Button::onClick() //
    {
        m_backgroundColorSave = this->borderSize_;
        this->borderSize_ += 2;

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
        borderSize_ = m_backgroundColorSave;
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
                space + ((m_image != nullptr) ? (m_image->getScrolledX() + m_image->getWidth())
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
            m_label = std::make_shared<Label>(0, 0, 0, 0);
            addChild(m_label);
        }
        this->m_label->setText(text);
        format();
    }

    void Button::setIcon(storage::Path path)
    {
        if (m_image == nullptr)
        {
            m_image = std::make_shared<Image>(path, 0, 10, 20, 20);
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
