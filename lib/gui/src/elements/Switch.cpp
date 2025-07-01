#include "Switch.hpp"

#include <Surface.hpp>
#include <cstdio>
#include <graphics.hpp>

namespace gui::elements
{
    Switch::Switch(const uint16_t x, const uint16_t y)
    {
        x_ = x;
        y_ = y;
        width_ = 40;
        height_ = 20;
        hasEvents_ = true;
    }

    Switch::~Switch() = default;

    void Switch::render()
    {
        surface_->clear(parent_ == nullptr ? COLOR_WHITE : parent_->getBackgroundColor());
        if (!m_state)
        {
            surface_->fillRoundRect(0, 0, width_, height_, 10, COLOR_GREY);
            surface_->fillRoundRect(4, 3, 14, 14, 7, COLOR_WHITE);
        }
        else
        {
            surface_->fillRoundRect(0, 0, width_, height_, 10, COLOR_BLACK);
            surface_->fillRoundRect(22, 3, 14, 14, 7, COLOR_WHITE);
        }
    }

    void Switch::onReleased()
    {
        setState(!m_state);
    }

    void Switch::setState(bool state)
    {
        this->m_state = state;
        localGraphicalUpdate();
    }

    bool Switch::getState()
    {
        return m_state;
    }
} // namespace gui::elements
