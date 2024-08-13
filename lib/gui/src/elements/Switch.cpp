#include "Switch.hpp"

#include <cstdio>
#include <graphics.hpp>
#include <Surface.hpp>

namespace gui::elements {
    Switch::Switch(const uint16_t x, const uint16_t y)
    {
        m_x = x;
        m_y = y;
        m_width = 40;
        m_height = 20;
        m_hasEvents = true;
    }

    Switch::~Switch() = default;

    void Switch::render()
    {
        m_surface->clear(COLOR_WHITE);
        if(!m_state)
        {
            m_surface->fillRoundRect(0, 0, m_width, m_height, 10, COLOR_GREY);
            m_surface->fillRoundRect(4, 3, 14, 14, 7, COLOR_WHITE);
        }
        else
        {
            m_surface->fillRoundRect(0, 0, m_width, m_height, 10, COLOR_BLACK);
            m_surface->fillRoundRect(22, 3, 14, 14, 7, COLOR_WHITE);
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
} // gui::elements