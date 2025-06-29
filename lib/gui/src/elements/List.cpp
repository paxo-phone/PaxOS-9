#include "List.hpp"

#include <iostream>
#include <threads.hpp>

namespace gui::elements
{
    // --- VERTICAL LIST IMPLEMENTATION (unchanged logic, cleaned) ---
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
        do_use_double_buffering = true;
    }

    VerticalList::~VerticalList() = default;

    void VerticalList::render()
    {
        m_surface->fillRect(0, 0, m_width, m_height, COLOR_WHITE);
    }

    void VerticalList::postRender()
    {
        // No scrollbar needed if list fits in view
        if (m_children.empty() ||
            (m_children.back()->m_y + m_children.back()->getHeight()) < getHeight())
        {
            return;
        }

        // Draw selection indicator for CENTER focus
        if (m_selectionFocus == SelectionFocus::CENTER && !m_children.empty())
        {
            int y = getHeight() / 2 - m_children[m_focusedIndex]->getHeight() / 2;
            int h = m_children[m_focusedIndex]->getHeight();
            m_surface->fillRect(0, y, 1, h, COLOR_BLACK);
            return;
        }

        // Draw scrollbar for UP or DOWN focus
        if (!m_children.empty() && m_focusedIndex < m_children.size())
        {
            uint16_t maxHeight = m_children.back()->m_y + m_children.back()->getHeight();
            int16_t barLen = (float) getHeight() * getHeight() / maxHeight;
            int16_t barY = 0;

            if (m_selectionFocus == SelectionFocus::UP)
            {
                // Bar position based on focused element's top
                barY = (float) m_children[m_focusedIndex]->m_y * getHeight() / maxHeight;
            }
            else if (m_selectionFocus == SelectionFocus::DOWN)
            {
                // Bar position based on focused element's bottom minus view height
                barY = (float) (m_children[m_focusedIndex]->m_y +
                                m_children[m_focusedIndex]->getHeight() - getHeight()) *
                       getHeight() / maxHeight;
                if (barY < 0)
                    barY = 0;
                if (barY + barLen > getHeight())
                    barY = getHeight() - barLen;
            }
            m_surface->fillRoundRect(getWidth() - 3, barY, 3, barLen, 1, COLOR_GREY);
        }
    }

    void VerticalList::add(ElementBase* widget)
    {
        m_verticalScrollEnabled = true;
        widget->setY(
            (m_children.size() != 0)
                ? (m_children.back()->m_y + m_children.back()->getHeight() + m_lineSpace)
                : (0)
        );
        this->addChild(widget);
    }

    void VerticalList::setIndex(int index)
    {
        if (index >= 0 && index < m_children.size())
        {
            if (!m_autoSelect)
                m_focusedIndex = index;
        }
    }

    void VerticalList::setIsSelected(bool selected)
    {
        isSelected = selected;
    }

    bool VerticalList::getIsSelected()
    {
        return isSelected;
    }

    void VerticalList::select(int index)
    {
        if (index >= 0 && index < m_children.size())
        {
            m_focusedIndex = index;
            ElementBase* oldSelection = this->getElementAt(m_oldFocusedIndex);
            if (oldSelection != nullptr)
                oldSelection->setBackgroundColor(m_backgroundColor);

            ElementBase* selection = this->getElementAt(index);
            if (selection != nullptr)
            {
                selection->setRadius(5);
                selection->setBackgroundColor(m_selectionColor);
            }

            m_oldFocusedIndex = m_focusedIndex;
            setIsSelected(true);
        }
    }

    void VerticalList::setSelectionColor(color_t color)
    {
        m_selectionColor = color;
    }

    void VerticalList::setSpaceLine(uint16_t y)
    {
        m_lineSpace = y;
    }

    void VerticalList::setAutoSelect(bool autoSelect)
    {
        m_autoSelect = autoSelect;
    }

    void VerticalList::updateFocusedIndex()
    {
        eventHandlerApp.setTimeout(
            new Callback<>(
                [&]()
                {
                    if (m_children.size() == 0)
                    {
                        m_focusedIndex = 0;
                        return;
                    }

                    m_verticalScroll = m_children[m_focusedIndex]->m_y;

                    switch (m_selectionFocus)
                    {
                    case SelectionFocus::UP:
                        m_verticalScroll = m_verticalScroll;
                        break;
                    case SelectionFocus::CENTER:
                        m_verticalScroll = m_verticalScroll - getHeight() / 2 +
                                           m_children[m_focusedIndex]->getHeight() / 2;
                        break;
                    case SelectionFocus::DOWN:
                        m_verticalScroll = m_verticalScroll - getHeight() +
                                           m_children[m_focusedIndex]->getHeight();
                        break;
                    }

                    localGraphicalUpdate();
                }
            ),
            0
        );
    }

    void VerticalList::onScroll(int16_t x, int16_t y)
    {
        if (!m_autoSelect)
        {
            if (m_children.size() == 0)
            {
                m_focusedIndex = 0;
                m_focusedElement = nullptr;
                m_verticalScroll = 0;
                return;
            }
            if (m_children[m_children.size() - 1]->m_y +
                    m_children[m_children.size() - 1]->getHeight() <
                getHeight())
            {
                m_verticalScroll = 0;
                return;
            }

            int16_t maxScroll =
                m_children.back()->m_y + m_children.back()->getHeight() - getHeight();

            if (m_verticalScroll - y < 0)
                m_verticalScroll = 0;
            else if (m_verticalScroll - y > maxScroll)
                m_verticalScroll = maxScroll;
            else
                m_verticalScroll -= y;
        }
        else
        {
            int16_t minScroll = 0;
            int16_t maxScroll =
                m_children.back()->m_y + m_children.back()->getHeight() - getHeight();

            if (m_selectionFocus == SelectionFocus::CENTER)
            {
                minScroll = -m_children.front()->getHeight() / 2 - getHeight() / 2;
                maxScroll =
                    m_children.back()->m_y + m_children.back()->getHeight() / 2 - getHeight() / 2;
                if (maxScroll < 0)
                    maxScroll = 0;
            }

            if (m_verticalScroll - y < minScroll)
                m_verticalScroll = minScroll;
            else if (m_verticalScroll - y > maxScroll)
                m_verticalScroll = maxScroll;
            else
                m_verticalScroll -= y;
        }

        ElementBase* element = nullptr;
        int focusedIndex = -1;

        for (int i = 0; i < m_children.size(); i++)
        {
            if ((m_selectionFocus == SelectionFocus::UP && m_children[i]->getY() >= 0) ||
                (m_selectionFocus == SelectionFocus::CENTER &&
                 m_children[i]->getY() + m_children[i]->getHeight() >= getHeight() / 2) ||
                (m_selectionFocus == SelectionFocus::DOWN &&
                 m_children[i]->getY() + m_children[i]->getHeight() >= getHeight()))
            {
                element = m_children[i];
                focusedIndex = i;
                break;
            }
        }

        if (element == nullptr && !m_children.empty())
        {
            element = m_children[0];
            focusedIndex = 0;
        }

        m_focusedElement = element;
        m_focusedIndex = focusedIndex;

        if (m_autoSelect && m_focusedElement != nullptr)
            select(m_focusedIndex);

        localGraphicalUpdate();
    }

    void VerticalList::onNotClicked()
    {
        if (m_autoSelect && m_focusedElement != nullptr)
        {
            if (m_selectionFocus == SelectionFocus::UP)
            {
                m_verticalScroll = m_focusedElement->m_y;
            }
            else if (m_selectionFocus == SelectionFocus::CENTER)
            {
                m_verticalScroll =
                    m_focusedElement->m_y - getHeight() / 2 + m_focusedElement->getHeight() / 2;
            }

            localGraphicalUpdate();
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

    // --- HORIZONTAL LIST REWRITTEN TO MATCH VERTICAL LIST MODEL ---
    HorizontalList::HorizontalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
        m_lineSpace = 25;
        m_selectionFocus = SelectionFocus::LEFT;
        m_selectionColor = COLOR_LIGHT_GREY;
        m_autoSelect = false;
        do_use_double_buffering = true;
    }

    HorizontalList::~HorizontalList() = default;

    void HorizontalList::render()
    {
        m_surface->fillRect(0, 0, m_width, m_height, COLOR_WHITE);
    }

    void HorizontalList::postRender()
    {
        if (m_selectionFocus == SelectionFocus::CENTER && m_children.size())
        {
            m_surface->fillRect(
                getWidth() / 2 - m_children[m_focusedIndex]->getWidth() / 2,
                0,
                m_children[m_focusedIndex]->getWidth(),
                1,
                COLOR_BLACK
            );
        }
        else if (m_children.size() && m_children.size() > m_focusedIndex)
        {
            uint16_t maxWidth = m_children.back()->m_x + m_children.back()->getWidth();
            int16_t cursor_size = (float) this->getWidth() * this->getWidth() / maxWidth;
            int16_t cursor_x =
                (float) m_children[m_focusedIndex]->m_x * this->getWidth() / maxWidth;

            m_surface->fillRoundRect(cursor_x, getHeight() - 3, cursor_size, 3, 1, COLOR_GREY);
        }
    }

    void HorizontalList::add(ElementBase* widget)
    {
        widget->setX(
            (m_children.size() != 0)
                ? (m_children.back()->m_x + m_children.back()->getWidth() + m_lineSpace)
                : (0)
        );
        this->addChild(widget);
    }

    void HorizontalList::setIndex(int index)
    {
        if (m_focusedIndex >= 0 && m_focusedIndex < m_children.size())
        {
            if (m_autoSelect)
            {
                // select(index);
            }
            else
            {
                m_focusedIndex = index;
                // updateFocusedIndex();
            }
        }
    }

    void HorizontalList::setIsSelected(bool selected)
    {
        isSelected = selected;
    }

    bool HorizontalList::getIsSelected()
    {
        return isSelected;
    }

    void HorizontalList::select(int index)
    {
        if (index >= 0 && index < m_children.size())
        {
            m_focusedIndex = index;
            ElementBase* oldSelection = this->getElementAt(m_oldFocusedIndex);
            if (oldSelection != nullptr)
                oldSelection->setBackgroundColor(m_backgroundColor);

            ElementBase* selection = this->getElementAt(index);
            if (selection != nullptr)
            {
                selection->setRadius(5);
                selection->setBackgroundColor(m_selectionColor);
            }

            m_oldFocusedIndex = m_focusedIndex;
            setIsSelected(true);
        }
    }

    void HorizontalList::setSelectionColor(color_t color)
    {
        m_selectionColor = color;
    }

    void HorizontalList::setSpaceLine(uint16_t y)
    {
        m_lineSpace = y;
    }

    void HorizontalList::setAutoSelect(bool autoSelect)
    {
        m_autoSelect = autoSelect;
    }

    void HorizontalList::updateFocusedIndex()
    {
        eventHandlerApp.setTimeout(
            new Callback<>(
                [&]()
                {
                    if (m_children.size() == 0)
                    {
                        m_focusedIndex = 0;
                        return;
                    }

                    m_horizontalScroll = m_children[m_focusedIndex]->m_x;
                    if (m_selectionFocus == SelectionFocus::CENTER)
                    {
                        m_horizontalScroll = m_horizontalScroll - getWidth() / 2 +
                                             m_children[m_focusedIndex]->getWidth() / 2;
                    }

                    localGraphicalUpdate();
                }
            ),
            0
        );
    }

    void HorizontalList::onScroll(int16_t x, int16_t y)
    {
        if (!m_autoSelect)
        {
            int16_t maxScroll = m_children.back()->m_x + m_children.back()->getWidth() - getWidth();

            if (m_horizontalScroll - x < 0)
                m_horizontalScroll = 0;
            else if (m_horizontalScroll - x > maxScroll)
                m_horizontalScroll = maxScroll;
            else
                m_horizontalScroll -= x;
        }
        else
        {
            int16_t minScroll = 0;
            int16_t maxScroll = m_children.back()->m_x + m_children.back()->getWidth() - getWidth();

            if (m_selectionFocus == SelectionFocus::CENTER)
            {
                minScroll = -m_children.front()->getWidth() / 2 - getWidth() / 2;
                maxScroll =
                    m_children.back()->m_x + m_children.back()->getWidth() / 2 - getWidth() / 2;
                if (maxScroll < 0)
                    maxScroll = 0;
            }

            if (m_horizontalScroll - x < minScroll)
                m_horizontalScroll = minScroll;
            else if (m_horizontalScroll - x > maxScroll)
                m_horizontalScroll = maxScroll;
            else
                m_horizontalScroll -= x;
        }

        ElementBase* element = nullptr;
        int focusedIndex = -1;

        for (int i = 0; i < m_children.size(); i++)
        {
            if (m_selectionFocus == SelectionFocus::LEFT && m_children[i]->getX() >= 0)
            {
                element = m_children[i];
                focusedIndex = i;
                break;
            }
            else if (m_selectionFocus == SelectionFocus::CENTER &&
                     m_children[i]->getX() + m_children[i]->getWidth() >= getWidth() / 2)
            {
                element = m_children[i];
                focusedIndex = i;
                break;
            }
        }

        if (element == nullptr && !m_children.empty())
        {
            element = m_children[0];
            focusedIndex = 0;
        }

        m_focusedElement = element;
        m_focusedIndex = focusedIndex;

        if (m_autoSelect && m_focusedElement != nullptr)
            select(m_focusedIndex);

        localGraphicalUpdate();
    }

    void HorizontalList::onNotClicked()
    {
        if (m_autoSelect && m_focusedElement != nullptr)
        {
            if (m_selectionFocus == SelectionFocus::LEFT)
            {
                m_horizontalScroll = m_focusedElement->m_x;
            }
            else if (m_selectionFocus == SelectionFocus::CENTER)
            {
                m_horizontalScroll =
                    m_focusedElement->m_x - getWidth() / 2 + m_focusedElement->getWidth() / 2;
            }

            localGraphicalUpdate();
        }
    }

    void HorizontalList::setSelectionFocus(SelectionFocus focus)
    {
        m_selectionFocus = focus;
        updateFocusedIndex();
    }

    int HorizontalList::getFocusedElement()
    {
        return m_focusedIndex;
    }
} // namespace gui::elements
