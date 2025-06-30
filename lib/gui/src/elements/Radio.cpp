#include "Radio.hpp"

#include "Image.hpp"

#include <Surface.hpp>
#include <cstdio>
#include <graphics.hpp>

namespace gui::elements
{

    Radio::Radio(const uint16_t x, const uint16_t y)
    {
        m_x = x;
        m_y = y;
        m_width = 20;
        m_height = 20;
        m_hasEvents = true;
    }

    Radio::~Radio() = default;

    void Radio::render()
    {
        m_surface->clear(COLOR_WHITE);

        m_surface->fillRoundRectWithBorder(0, 0, m_width, m_height, 10, 2, COLOR_WHITE, COLOR_DARK);

        if (m_state)
            m_surface->fillRoundRect(4, 4, m_width - 8, m_height - 8, 10, COLOR_DARK);
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
