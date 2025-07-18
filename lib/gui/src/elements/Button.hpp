#ifndef BUTTON_HPP
#define BUTTON_HPP

#include "../ElementBase.hpp"
#include "Image.hpp"
#include "Label.hpp"

#include <filestream.hpp>

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
        void onClick();
        void onNotClicked();

        void setBorderColor(color_t color)
        {
            if (m_label == nullptr)
                return;

            m_label->setBorderColor(color);
        }

        void setFontSize(uint16_t r)
        {
            if (m_label == nullptr)
                return;

            m_label->setFontSize(r);
        }

        void setBackgroundColor(color_t color)
        {
            if (m_label == nullptr)
                return;

            m_label->setBackgroundColor(color);
        }

        void setVerticalAlignment(Label::Alignement alignment)
        {
            if (m_label == nullptr)
                return;

            m_label->setVerticalAlignment(alignment);
        }

        void setHorizontalAlignment(Label::Alignement alignment)
        {
            if (m_label == nullptr)
                return;

            m_label->setHorizontalAlignment(alignment);
        }

        void setTextColor(color_t color)
        {
            if (m_label == nullptr)
                return;

            m_label->setTextColor(color);
        }

        std::string getText();

        void setTheme(bool value);

      private:
        std::shared_ptr<Label> m_label;
        std::shared_ptr<Image> m_image;
        bool m_theme;
        color_t m_backgroundColorSave;
    };
} // namespace gui::elements

#endif // BUTTON_HPP
