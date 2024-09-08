#ifndef LIST_HPP
#define LIST_HPP

#include "../ElementBase.hpp"

namespace gui::elements
{
    class VerticalList final : public ElementBase
    {
    public:
        VerticalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~VerticalList() override;

        void render() override;
        void postRender();

        void add(ElementBase* widget);
        void setIndex(int index);

        void setSpaceLine(uint16_t y);

        void updateFocusedIndex();
        void onScrollUp();
        void onScrollDown();

        enum SelectionFocus
        {
            UP,
            CENTER
        };

        void setSelectionFocus(SelectionFocus focus);
        int getFocusedElement();
        void select(int index);
        void setSelectionColor(color_t color);
        void setAutoSelect(bool autoSelect);


    private:
        int16_t m_focusedIndex = 0;
        int16_t m_oldFocusedIndex = 0;

        uint16_t m_lineSpace = 0;
        SelectionFocus m_selectionFocus = SelectionFocus::UP;
        color_t m_selectionColor;
        bool m_autoSelect;

    };

    class HorizontalList final : public ElementBase
    {
    public:
        HorizontalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~HorizontalList() override;

        void render() override;

        void add(ElementBase* widget);

        void setSpaceLine(uint16_t y);

    private:
        uint16_t m_lineSpace = 0;
    };
} // gui::elements

#endif //LIST_HPP
