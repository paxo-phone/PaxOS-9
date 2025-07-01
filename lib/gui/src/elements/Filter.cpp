#include "Filter.hpp"

#include "graphics.hpp"

#include <iostream>
#include <standby.hpp>

namespace gui::elements
{
    Filter::Filter(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height)
    {
        this->x_ = x;
        this->y_ = y;
        this->width_ = width;
        this->height_ = height;

        m_screenSurface = std::make_shared<graphics::Surface>(width_, height_);
    }

    void Filter::render()
    {
        std::cout << "RENDER FILTER" << std::endl;

        surface_->pushSurface(m_screenSurface.get(), 0, 0);
    }

    void Filter::apply() const
    {
        std::cout << "APPLY FILTER" << std::endl;

        StandbyMode::triggerPower();

        // Get screen
        LGFX* lcd = graphics::getLCD();

        // Copy screen zone to buffer
        lcd->readRect(
            x_,
            y_,
            width_,
            height_,
            static_cast<uint16_t*>(m_screenSurface->getBuffer())
        );

        // Apply filter
        m_screenSurface->applyFilter(graphics::Surface::LIGHTEN, 100);
        m_screenSurface->applyFilter(graphics::Surface::DARKEN, 200);
        m_screenSurface->applyFilter(graphics::Surface::BLUR, 3);
    }

    Filter::~Filter() = default;
} // namespace gui::elements
