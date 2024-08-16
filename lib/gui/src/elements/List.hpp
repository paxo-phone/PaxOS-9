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

        void add(ElementBase* widget);
        void clear();

        void setIndex(int index);

        void setSpaceLine(uint16_t y);

        void updateFocusedIndex();
        void onScrollUp();
        void onScrollDown();

    private:
        int16_t m_focusedIndex = 0;
        uint16_t m_lineSpace = 0;
    };

    class HorizontalList final : public ElementBase
    {
    public:
        HorizontalList(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~HorizontalList() override;

        void render() override;
        void add(ElementBase* widget);
        void clear();
        void setSpaceLine(uint16_t y);

    private:
        uint16_t m_lineSpace = 0;
    };
} // gui::elements

#endif //LIST_HPP
