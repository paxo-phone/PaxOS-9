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
        m_selectionFocus = SelectionFocus::UP;
        m_selectionColor = COLOR_LIGHT_GREY;
        m_autoSelect = false;
    }

    VerticalList::~VerticalList() = default;

    void VerticalList::render()
    {
        m_surface->fillRect(0, 0, m_width, m_height, COLOR_WHITE);
    }

    void VerticalList::postRender()
    {
        if(m_selectionFocus == SelectionFocus::CENTER && m_children.size())
        {
            m_surface->fillRect(0, getHeight()/2 - m_children[m_focusedIndex]->getHeight()/2, 1, m_children[m_focusedIndex]->getHeight(), COLOR_BLACK);
        }
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
            if (m_autoSelect) {
               select(index);
            }
            else {
               m_focusedIndex = index;
               updateFocusedIndex();
            }
        }
        
    }

    void VerticalList::setIsSelected(bool selected) {
        isSelected = selected;
    }

    bool VerticalList::getIsSelected() {
        return isSelected;
    }


    void VerticalList::select(int index)
    {
        if(index >= 0 && index < m_children.size())
        {
            m_focusedIndex = index;
            ElementBase *oldSelection = this->getElementAt(m_oldFocusedIndex);
            if (oldSelection != nullptr)
                oldSelection->setBackgroundColor(m_backgroundColor);

            ElementBase *selection = this->getElementAt(index);
            if (selection != nullptr)
                selection->setBackgroundColor(m_selectionColor);

            m_oldFocusedIndex = m_focusedIndex;
            setIsSelected(true);

            updateFocusedIndex();
        }
    }
    
    void VerticalList::setSelectionColor(color_t color){
        m_selectionColor = color;
    }

    void VerticalList::setSpaceLine(uint16_t y)
    {
        m_lineSpace = y;
    }

    void VerticalList::setAutoSelect(bool autoSelect)  {
        m_autoSelect = autoSelect;
    }


    void VerticalList::updateFocusedIndex()
    {
        eventHandlerApp.setTimeout(new Callback<>([&](){
            //std::cout << "updateFocusedIndex" << std::endl;
            if(m_children.size() == 0)
            {
                m_focusedIndex = 0;
                return;
            }
            
            m_verticalScroll = m_children[m_focusedIndex]->m_y;
            if(m_selectionFocus == SelectionFocus::CENTER)
                m_verticalScroll = m_verticalScroll - getHeight() / 2 + m_children[m_focusedIndex]->getHeight() / 2;
            
            localGraphicalUpdate();
            //std::cout << "updateFocusedIndex end: " << m_selectionFocus << std::endl;

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
            if (m_autoSelect) {
                select(m_focusedIndex - 1);
            }
            else {
                m_focusedIndex--;
                updateFocusedIndex();
            }
        }
    }
    
    void VerticalList::onScrollDown()
    {
        if(m_focusedIndex+1 != m_children.size())
        {
            if (m_autoSelect) {
                select(m_focusedIndex + 1);
            }
            else {
                m_focusedIndex++;
                updateFocusedIndex();
            }
        }
    }

    void VerticalList::setSelectionFocus(SelectionFocus focus)
    {
        m_selectionFocus = focus;
        updateFocusedIndex();
    }

    int VerticalList::getFocusedElement()
    {
        return m_focusedIndex;
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