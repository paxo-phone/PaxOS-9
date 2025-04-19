#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../ElementBase.hpp"
#include <filestream.hpp>

#include "Label.hpp"
#include "Image.hpp"

#define BUTTON_BLACK 0
#define BUTTON_WHITE 1

namespace gui::elements
{
    /**
     * @brief Clickable widget containing a label or/and an image.
     *
     * This widget can contain a label or/and an image and is clickable.
     * It follows the theme designed by Paxo on figma.
     *
     * @todo Move source code to .cpp.
     */
    class Button final : public ElementBase
    {
    public:
        Button(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~Button() override;

        void format(); // recalculer les coordonées et tailles des sous widgets
        void render() override;

        void setText(std::string text);
        void setIcon(storage::Path path);

        void setBorderColor(color_t color) {
            if (m_label == nullptr) {
                return;
            }

            m_label->setBorderColor(color);
        }
        void setFontSize(uint16_t r) {
            if (m_label == nullptr) {
                return;
            }

            m_label->setFontSize(r);
        }
    
        void setBackgroundColor(color_t color) {
            if (m_label == nullptr) {
                return;
            }

            m_label->setBackgroundColor(color);
        }

        void setVerticalAlignment(Label::Alignement alignment) {
            if (m_label == nullptr) {
                return;
            }

            m_label->setVerticalAlignment(alignment);
        }
        void setHorizontalAlignment(Label::Alignement alignment) {
            if (m_label == nullptr) {
                return;
            }

            m_label->setHorizontalAlignment(alignment);
        }
        void setTextColor(color_t color) {
            if (m_label == nullptr) {
                return;
            }

            m_label->setTextColor(color);
        }


        std::string getText();

        void setTheme(bool value);

    private:
        Label* m_label;
        Image* m_image;
        bool m_theme;
    };
} // gui::elements

#endif //BUTTON_HPP
