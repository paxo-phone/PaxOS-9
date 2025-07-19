#ifndef LIST_HPP
#define LIST_HPP

#include "../ElementBase.hpp"

namespace gui::elements
{
    /**
     * @brief Can contain a list of elements and is scrollable and is memory efficient
     **/
    class VerticalList final : public ElementBase
    {
      public:
        VerticalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~VerticalList();

        void render() override;
        void postRender();

        void add(ElementBase* widget);
        void setIndex(int index);
        void setSpaceLine(uint16_t y);
        void updateFocusedIndex();
        void onScroll(int16_t x, int16_t y);
        void onNotClicked();

        enum SelectionFocus
        {
            UP,
            CENTER,
            DOWN
        };

        void setSelectionFocus(SelectionFocus focus);
        int getFocusedElement();
        void select(int index);
        void setSelectionColor(color_t color);
        void setAutoSelect(bool autoSelect);
        void setIsSelected(bool autoSelect);
        bool getIsSelected();

      private:
        int16_t m_focusedIndex = 0;
        int16_t m_oldFocusedIndex = 0;
        ElementBase* m_focusedElement = nullptr;

        uint16_t m_lineSpace = 0;
        SelectionFocus m_selectionFocus = SelectionFocus::UP;
        color_t m_selectionColor;
        bool m_autoSelect;
        bool isSelected = false;
    };

    class HorizontalList final : public ElementBase
    {
      public:
        HorizontalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~HorizontalList();

        void render() override;
        void postRender();

        void add(ElementBase* widget);
        void setIndex(int index);
        void setSpaceLine(uint16_t y);
        void updateFocusedIndex();
        void onScroll(int16_t x, int16_t y);
        void onNotClicked();

        enum SelectionFocus
        {
            LEFT,
            CENTER
        };

        void setSelectionFocus(SelectionFocus focus);
        int getFocusedElement();
        void select(int index);
        void setSelectionColor(color_t color);
        void setAutoSelect(bool autoSelect);
        void setIsSelected(bool autoSelect);
        bool getIsSelected();

      private:
        int16_t m_focusedIndex = 0;
        int16_t m_oldFocusedIndex = 0;
        ElementBase* m_focusedElement = nullptr;

        uint16_t m_lineSpace = 0;
        SelectionFocus m_selectionFocus = SelectionFocus::LEFT;
        color_t m_selectionColor;
        bool m_autoSelect;
        bool isSelected = false;
    };
} // namespace gui::elements

#endif // LIST_HPP
