//
// Created by Charles on 17/01/2024.
//

#include "Box.hpp"

#include <cstdio>
#include <graphics.hpp>
#include <Surface.hpp>

namespace gui::elements {
    Box::Box(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height) :
        r(0),
        g(0),
        b(0)
    {
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
    }

    Box::~Box() = default;

    void Box::render()
    {
        m_surface->setColor(r, g, b);
        m_surface->fillRect(0, 0, m_width, m_height);
    }

    void Box::getColor(uint8_t* r, uint8_t* g, uint8_t* b) const
    {
        *r = this->r;
        *g = this->g;
        *b = this->b;
    }

    void Box::setColor(const uint8_t r, const uint8_t g, const uint8_t b)
    {
        this->r = r;
        this->g = g;
        this->b = b;
    }

} // gui::elements