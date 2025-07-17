#include "Filter.hpp"

#include "graphics.hpp"

#include <iostream>
#include <standby.hpp>

namespace gui::elements
{
    Filter::Filter(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height)
    {
        this->m_x = x;
        this->m_y = y;
        this->m_width = width;
        this->m_height = height;

        m_screenSurface = std::make_shared<graphics::Surface>(m_width, m_height);
    }

    void Filter::render()
    {
        std::cout << "RENDER FILTER" << std::endl;

        m_surface->pushSurface(m_screenSurface.get(), 0, 0);
    }

    void Filter::apply() const
    {
        std::cout << "APPLY FILTER" << std::endl;

        StandbyMode::triggerPower();

        // Get screen
        LGFX* lcd = graphics::getLCD();

        // Copy screen zone to buffer
        lcd->readRect(
            m_x,
            m_y,
            m_width,
            m_height,
            static_cast<uint16_t*>(m_screenSurface->getBuffer())
        );

        // Apply filter
        m_screenSurface->applyFilter(graphics::Surface::LIGHTEN, 100);
        m_screenSurface->applyFilter(graphics::Surface::DARKEN, 200);
        m_screenSurface->applyFilter(graphics::Surface::BLUR, 3);
    }

    Filter::~Filter() = default;
} // namespace gui::elements
