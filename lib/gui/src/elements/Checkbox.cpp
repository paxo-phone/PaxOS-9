#include "Checkbox.hpp"

#include "Image.hpp"

#include <Surface.hpp>
#include <cstdio>
#include <graphics.hpp>

namespace gui::elements
{

    Checkbox::Checkbox(const uint16_t x, const uint16_t y)
    {
        x_ = x;
        y_ = y;
        width_ = 20;
        height_ = 20;
        hasEvents_ = true;

        m_tickIcon = new Image(storage::Path("tick.png"), 3, 4, 12, 8, COLOR_DARK);
        m_tickIcon->load();
        this->addChild(m_tickIcon);
        m_tickIcon->disable();

        // relier le released du tickicon au released de la checkbox
    }

    Checkbox::~Checkbox() = default;

    void Checkbox::render()
    {
        surface_->clear(COLOR_WHITE);

        if (m_state)
        {
            surface_->fillRoundRect(0, 0, width_, height_, 4, COLOR_DARK);
            m_tickIcon->enable();
        }
        else
        {
            surface_
                ->fillRoundRectWithBorder(0, 0, width_, height_, 4, 2, COLOR_WHITE, COLOR_DARK);
            m_tickIcon->disable();
        }
    }

    void Checkbox::onReleased()
    {
        setState(!m_state);
    }

    void Checkbox::setState(bool state)
    {
        this->m_state = state;
        localGraphicalUpdate();
    }

    bool Checkbox::getState()
    {
        return m_state;
    }
} // namespace gui::elements
