#include "Input.hpp"

#include <iostream>

namespace gui::elements
{
    Input::Input(uint16_t x, uint16_t y)
    {
        m_x = x;
        m_y = y;
        m_width = INPUT_WIDTH;
        m_height = INPUT_HEIGHT;

        m_title = new Label(0, 0, 250, 18);
        m_title->setFontSize(16);
        m_title->setTextColor(COLOR_GREY);
        this->addChild(m_title);

        m_text = new Label(0, 20, 231, 18);
        m_text->setFontSize(18);
        m_text->setTextColor(COLOR_GREY);
        this->addChild(m_text);

        m_clear = new Image(storage::Path("/icon.png"), 231, 20, 18, 18);
        m_clear->load();
        this->addChild(m_clear);
    }

    Input::~Input() = default;

    void Input::render()
    {
        m_surface->clear(COLOR_WHITE);
        m_surface->fillRect(0, getHeight() - 2, getWidth(), 2, COLOR_DARK);
    }

    void Input::widgetUpdate()
    {
        if (m_clear->isTouched())
            setText("");
    }

    void Input::setText(const std::string& text)
    {
        if (text.length() != 0)
        {
            m_hasText = true;
            m_text->setText(text);
            m_text->setTextColor(COLOR_DARK);
        }
        else
        {
            m_hasText = false;
            m_text->setText(m_placeHolder);
            m_text->setTextColor(COLOR_GREY);
        }

        localGraphicalUpdate();
    }

    void Input::setPlaceHolder(const std::string& text)
    {
        this->m_placeHolder = text;
        if (m_text->getText().length() == 0)
            m_text->setText(m_placeHolder);

        localGraphicalUpdate();
    }

    void Input::setTitle(const std::string& text)
    {
        m_title->setText(text);

        localGraphicalUpdate();
    }

    std::string Input::getText()
    {
        if (m_hasText)
            return m_text->getText();
        else
            return "";
    }

    std::string Input::getPlaceHolder()
    {
        return m_placeHolder;
    }

    std::string Input::getTitle()
    {
        return m_title->getText();
    }
} // namespace gui::elements
