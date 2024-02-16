#include "List.hpp"
#include <iostream>

namespace gui::elements
{
    VerticalList::VerticalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
        m_lineSpace = 25;
    }

    VerticalList::~VerticalList() = default;

    void VerticalList::render()
    {
        m_surface->clear(COLOR_WHITE);
    }

    void VerticalList::add(ElementBase* widget)
    {
        widget->setX(0);
        widget->setY( (m_children.size() != 0) ? (m_children[m_children.size()-1]->getY() + m_children[m_children.size()-1]->getHeight() + m_lineSpace) : (0));
        this->addChild(widget);
        this->setHeight(widget->getY()+widget->getHeight());
    }

    void VerticalList::setSpaceLine(uint16_t y)
    {
        m_lineSpace = y;
    }

    HorizontalList::HorizontalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
        m_lineSpace = 25;
    }

    HorizontalList::~HorizontalList() = default;

    void HorizontalList::render()
    {
        m_surface->clear(COLOR_WHITE);
    }

    void HorizontalList::add(ElementBase* widget)
    {
        widget->setX((m_children.size() != 0) ? (m_children[m_children.size()-1]->getX() + m_children[m_children.size()-1]->getWidth() + m_lineSpace) : (0));
        widget->setY(0);
        this->addChild(widget);
        this->setWidth(widget->getX()+widget->getWidth());
    }

    void HorizontalList::setSpaceLine(uint16_t y)
    {
        m_lineSpace = y;
    }
}