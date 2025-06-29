#include "Checkbox.hpp"

#include "Image.hpp"

#include <Surface.hpp>
#include <cstdio>
#include <graphics.hpp>

namespace gui::elements
{

    Checkbox::Checkbox(const uint16_t x, const uint16_t y)
    {
        m_x = x;
        m_y = y;
        m_width = 20;
        m_height = 20;
        m_hasEvents = true;

        m_tickIcon = new Image(storage::Path("tick.png"), 3, 4, 12, 8, COLOR_DARK);
        m_tickIcon->load();
        this->addChild(m_tickIcon);
        m_tickIcon->disable();

        // relier le released du tickicon au released de la checkbox
    }

    Checkbox::~Checkbox() = default;

    void Checkbox::render()
    {
        m_surface->clear(COLOR_WHITE);

        if (m_state)
        {
            m_surface->fillRoundRect(0, 0, m_width, m_height, 4, COLOR_DARK);
            m_tickIcon->enable();
        }
        else
        {
            m_surface
                ->fillRoundRectWithBorder(0, 0, m_width, m_height, 4, 2, COLOR_WHITE, COLOR_DARK);
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
