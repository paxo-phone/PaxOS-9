#include "List.hpp"

#include <iostream>
#include <threads.hpp>

namespace gui::elements
{
    // --- VERTICAL LIST IMPLEMENTATION (unchanged logic, cleaned) ---
    VerticalList::VerticalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
        m_lineSpace = 25;
        verticalScrollEnabled_ = true;
        hasEvents_ = true;
        m_selectionFocus = SelectionFocus::UP;
        m_selectionColor = COLOR_LIGHT_GREY;
        m_autoSelect = false;
        useDoubleBuffering_ = true;
    }

    VerticalList::~VerticalList() = default;

    void VerticalList::render()
    {
        surface_->fillRect(0, 0, width_, height_, COLOR_WHITE);
    }

    void VerticalList::postRender()
    {
        // No scrollbar needed if list fits in view
        if (children_.empty() ||
            (children_.back()->getY() + children_.back()->getHeight()) < getHeight())
        {
            return;
        }

        // Draw selection indicator for CENTER focus
        if (m_selectionFocus == SelectionFocus::CENTER && !children_.empty())
        {
            int y = getHeight() / 2 - children_[m_focusedIndex]->getHeight() / 2;
            int h = children_[m_focusedIndex]->getHeight();
            surface_->fillRect(0, y, 1, h, COLOR_BLACK);
            return;
        }

        // Draw scrollbar for UP or DOWN focus
        if (!children_.empty() && m_focusedIndex < children_.size())
        {
            uint16_t maxHeight = children_.back()->getY() + children_.back()->getHeight();
            int16_t barLen = (float) getHeight() * getHeight() / maxHeight;
            int16_t barY = 0;

            if (m_selectionFocus == SelectionFocus::UP)
            {
                // Bar position based on focused element's top
                barY = (float) children_[m_focusedIndex]->getY() * getHeight() / maxHeight;
            }
            else if (m_selectionFocus == SelectionFocus::DOWN)
            {
                // Bar position based on focused element's bottom minus view height
                barY = (float) (children_[m_focusedIndex]->getY() +
                                children_[m_focusedIndex]->getHeight() - getHeight()) *
                       getHeight() / maxHeight;
                if (barY < 0)
                    barY = 0;
                if (barY + barLen > getHeight())
                    barY = getHeight() - barLen;
            }
            surface_->fillRoundRect(getWidth() - 3, barY, 3, barLen, 1, COLOR_GREY);
        }
    }

    void VerticalList::add(ElementBase* widget)
    {
        verticalScrollEnabled_ = true;
        widget->setY(
            (children_.size() != 0)
                ? (children_.back()->getY() + children_.back()->getHeight() + m_lineSpace)
                : (0)
        );
        this->addChild(widget);
    }

    void VerticalList::setIndex(int index)
    {
        if (index >= 0 && index < children_.size())
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
        if (index >= 0 && index < children_.size())
        {
            m_focusedIndex = index;
            ElementBase* oldSelection = this->getElementAt(m_oldFocusedIndex);
            if (oldSelection != nullptr)
                oldSelection->setBackgroundColor(backgroundColor_);

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
                    if (children_.size() == 0)
                    {
                        m_focusedIndex = 0;
                        return;
                    }

                    verticalScroll_ = children_[m_focusedIndex]->getY();

                    switch (m_selectionFocus)
                    {
                    case SelectionFocus::UP:
                        verticalScroll_ = verticalScroll_;
                        break;
                    case SelectionFocus::CENTER:
                        verticalScroll_ = verticalScroll_ - getHeight() / 2 +
                                          children_[m_focusedIndex]->getHeight() / 2;
                        break;
                    case SelectionFocus::DOWN:
                        verticalScroll_ =
                            verticalScroll_ - getHeight() + children_[m_focusedIndex]->getHeight();
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
            if (children_.size() == 0)
            {
                m_focusedIndex = 0;
                m_focusedElement = nullptr;
                verticalScroll_ = 0;
                return;
            }
            if (children_[children_.size() - 1]->getY() +
                    children_[children_.size() - 1]->getHeight() <
                getHeight())
            {
                verticalScroll_ = 0;
                return;
            }

            int16_t maxScroll =
                children_.back()->getY() + children_.back()->getHeight() - getHeight();

            if (verticalScroll_ - y < 0)
                verticalScroll_ = 0;
            else if (verticalScroll_ - y > maxScroll)
                verticalScroll_ = maxScroll;
            else
                verticalScroll_ -= y;
        }
        else
        {
            int16_t minScroll = 0;
            int16_t maxScroll =
                children_.back()->getY() + children_.back()->getHeight() - getHeight();

            if (m_selectionFocus == SelectionFocus::CENTER)
            {
                minScroll = -children_.front()->getHeight() / 2 - getHeight() / 2;
                maxScroll =
                    children_.back()->getY() + children_.back()->getHeight() / 2 - getHeight() / 2;
                if (maxScroll < 0)
                    maxScroll = 0;
            }

            if (verticalScroll_ - y < minScroll)
                verticalScroll_ = minScroll;
            else if (verticalScroll_ - y > maxScroll)
                verticalScroll_ = maxScroll;
            else
                verticalScroll_ -= y;
        }

        ElementBase* element = nullptr;
        int focusedIndex = -1;

        for (int i = 0; i < children_.size(); i++)
        {
            if ((m_selectionFocus == SelectionFocus::UP && children_[i]->getScrolledY() >= 0) ||
                (m_selectionFocus == SelectionFocus::CENTER &&
                 children_[i]->getScrolledY() + children_[i]->getHeight() >= getHeight() / 2) ||
                (m_selectionFocus == SelectionFocus::DOWN &&
                 children_[i]->getScrolledY() + children_[i]->getHeight() >= getHeight()))
            {
                element = children_[i];
                focusedIndex = i;
                break;
            }
        }

        if (element == nullptr && !children_.empty())
        {
            element = children_[0];
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
                verticalScroll_ = m_focusedElement->getY();
            }
            else if (m_selectionFocus == SelectionFocus::CENTER)
            {
                verticalScroll_ =
                    m_focusedElement->getY() - getHeight() / 2 + m_focusedElement->getHeight() / 2;
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
        x_ = x;
        y_ = y;
        width_ = width;
        height_ = height;
        m_lineSpace = 25;
        m_selectionFocus = SelectionFocus::LEFT;
        m_selectionColor = COLOR_LIGHT_GREY;
        m_autoSelect = false;
        useDoubleBuffering_ = true;
    }

    HorizontalList::~HorizontalList() = default;

    void HorizontalList::render()
    {
        surface_->fillRect(0, 0, width_, height_, COLOR_WHITE);
    }

    void HorizontalList::postRender()
    {
        if (m_selectionFocus == SelectionFocus::CENTER && children_.size())
        {
            surface_->fillRect(
                getWidth() / 2 - children_[m_focusedIndex]->getWidth() / 2,
                0,
                children_[m_focusedIndex]->getWidth(),
                1,
                COLOR_BLACK
            );
        }
        else if (children_.size() && children_.size() > m_focusedIndex)
        {
            uint16_t maxWidth = children_.back()->getX() + children_.back()->getWidth();
            int16_t cursor_size = (float) this->getWidth() * this->getWidth() / maxWidth;
            int16_t cursor_x =
                (float) children_[m_focusedIndex]->getX() * this->getWidth() / maxWidth;

            surface_->fillRoundRect(cursor_x, getHeight() - 3, cursor_size, 3, 1, COLOR_GREY);
        }
    }

    void HorizontalList::add(ElementBase* widget)
    {
        widget->setX(
            (children_.size() != 0)
                ? (children_.back()->getX() + children_.back()->getWidth() + m_lineSpace)
                : (0)
        );
        this->addChild(widget);
    }

    void HorizontalList::setIndex(int index)
    {
        if (m_focusedIndex >= 0 && m_focusedIndex < children_.size())
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
        if (index >= 0 && index < children_.size())
        {
            m_focusedIndex = index;
            ElementBase* oldSelection = this->getElementAt(m_oldFocusedIndex);
            if (oldSelection != nullptr)
                oldSelection->setBackgroundColor(backgroundColor_);

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
                    if (children_.size() == 0)
                    {
                        m_focusedIndex = 0;
                        return;
                    }

                    horizontalScroll_ = children_[m_focusedIndex]->getX();
                    if (m_selectionFocus == SelectionFocus::CENTER)
                    {
                        horizontalScroll_ = horizontalScroll_ - getWidth() / 2 +
                                            children_[m_focusedIndex]->getWidth() / 2;
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
            int16_t maxScroll =
                children_.back()->getX() + children_.back()->getWidth() - getWidth();

            if (horizontalScroll_ - x < 0)
                horizontalScroll_ = 0;
            else if (horizontalScroll_ - x > maxScroll)
                horizontalScroll_ = maxScroll;
            else
                horizontalScroll_ -= x;
        }
        else
        {
            int16_t minScroll = 0;
            int16_t maxScroll =
                children_.back()->getX() + children_.back()->getWidth() - getWidth();

            if (m_selectionFocus == SelectionFocus::CENTER)
            {
                minScroll = -children_.front()->getWidth() / 2 - getWidth() / 2;
                maxScroll =
                    children_.back()->getX() + children_.back()->getWidth() / 2 - getWidth() / 2;
                if (maxScroll < 0)
                    maxScroll = 0;
            }

            if (horizontalScroll_ - x < minScroll)
                horizontalScroll_ = minScroll;
            else if (horizontalScroll_ - x > maxScroll)
                horizontalScroll_ = maxScroll;
            else
                horizontalScroll_ -= x;
        }

        ElementBase* element = nullptr;
        int focusedIndex = -1;

        for (int i = 0; i < children_.size(); i++)
        {
            if (m_selectionFocus == SelectionFocus::LEFT && children_[i]->getScrolledX() >= 0)
            {
                element = children_[i];
                focusedIndex = i;
                break;
            }
            else if (m_selectionFocus == SelectionFocus::CENTER &&
                     children_[i]->getScrolledX() + children_[i]->getWidth() >= getWidth() / 2)
            {
                element = children_[i];
                focusedIndex = i;
                break;
            }
        }

        if (element == nullptr && !children_.empty())
        {
            element = children_[0];
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
                horizontalScroll_ = m_focusedElement->getX();
            }
            else if (m_selectionFocus == SelectionFocus::CENTER)
            {
                horizontalScroll_ =
                    m_focusedElement->getX() - getWidth() / 2 + m_focusedElement->getWidth() / 2;
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
