#include "List.hpp"
#include <iostream>
#include <threads.hpp>

namespace gui::elements
{
    VerticalList::VerticalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
        m_lineSpace = 25;
        m_verticalScrollEnabled = true;
        m_hasEvents = true;
    }

    VerticalList::~VerticalList() = default;

    void VerticalList::render()
    {
        m_surface->fillRect(0, 0, m_width, m_height, COLOR_WHITE);
    }

    void VerticalList::add(ElementBase* widget)
    {
        m_verticalScrollEnabled = true;
        widget->setY( (m_children.size() != 0) ? (m_children.back()->m_y + m_children.back()->getHeight() + m_lineSpace) : (0));
        this->addChild(widget);
    }

    void VerticalList::setIndex(int index)
    {
        if(m_focusedIndex >= 0 && m_focusedIndex < m_children.size())
        {
            m_focusedIndex = index;
            updateFocusedIndex();
        }
    }

    void VerticalList::setSpaceLine(uint16_t y)
    {
        m_lineSpace = y;
    }

    void VerticalList::updateFocusedIndex()
    {
        eventHandlerApp.setTimeout(new Callback<>([&](){
            if(m_children.size() == 0)
            {
                m_focusedIndex = 0;
                return;
            }
            
            m_verticalScroll = m_children[m_focusedIndex]->m_y;
            localGraphicalUpdate();

            // for (int i = 0; i < m_children.size(); i++)
            // {
            //     if (m_children[i]->getY() + m_children[i]->getHeight() < 0 || m_children[i]->getY() > m_height)
            //     {
            //         m_children[i]->free();
            //     }
            // }
        }), 0);
    }

    void VerticalList::onScrollUp()
    {
        if(m_focusedIndex != 0)
        {
            m_focusedIndex--;
            updateFocusedIndex();
        }
    }
    
    void VerticalList::onScrollDown()
    {
        if(m_focusedIndex+1 != m_children.size())
        {
            m_focusedIndex++;
            updateFocusedIndex();
        }
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
        this->addChild(widget);
        this->setWidth(widget->getX()+widget->getWidth());
    }

    void HorizontalList::setSpaceLine(uint16_t y)
    {
        m_lineSpace = y;
    }
}