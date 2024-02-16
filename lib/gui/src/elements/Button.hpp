#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../ElementBase.hpp"

#include "Label.hpp"
#include "Image.hpp"

#define BUTTON_BLACK 0
#define BUTTON_WHITE 1

namespace gui::elements
{
    class Button final : public ElementBase
    {
    public:
        Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~Button() override;

        void format(); // recalculer les coordonées et tailles des sous widgets
        void render() override;

        void setLabel(Label* label);
        void setImage(Image* image);

        void setTheme(bool value);

    private:
        Label* m_label;
        Image* m_image;
        bool m_theme;
    };
} // gui::elements

#endif //BUTTON_HPP
