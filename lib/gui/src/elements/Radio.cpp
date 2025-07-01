#include "Radio.hpp"

#include "Image.hpp"

#include <Surface.hpp>
#include <cstdio>
#include <graphics.hpp>

namespace gui::elements
{

    Radio::Radio(const uint16_t x, const uint16_t y)
    {
        x_ = x;
        y_ = y;
        width_ = 20;
        height_ = 20;
        hasEvents_ = true;
    }

    Radio::~Radio() = default;

    void Radio::render()
    {
        surface_->clear(COLOR_WHITE);

        surface_->fillRoundRectWithBorder(0, 0, width_, height_, 10, 2, COLOR_WHITE, COLOR_DARK);

        if (m_state)
            surface_->fillRoundRect(4, 4, width_ - 8, height_ - 8, 10, COLOR_DARK);
    }

    void Radio::onReleased()
    {
        setState(!m_state);
    }

    void Radio::setState(bool state)
    {
        this->m_state = state;
        localGraphicalUpdate();
    }

    bool Radio::getState()
    {
        return m_state;
    }
} // namespace gui::elements
