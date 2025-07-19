#ifndef LABEL_HPP
#define LABEL_HPP

#include "../ElementBase.hpp"

#define LINE_SPACING 2

#define LABEL_SMALL 16
#define LABEL_MEDIUM 24
#define LABEL_BIG 36

namespace gui::elements
{
    /**
     * @brief Widget label displays text
     **/
    class Label final : public ElementBase
    {
      public:
        Label(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~Label();

        enum Alignement
        {
            CENTER,
            LEFT,
            RIGHT,
            UP,
            DOWN
        };

        void render() override;

        void setText(const std::string& text);
        std::string getText() const;
        void setTextColor(color_t color);
        void setFontSize(uint16_t fontSize);

        void setHorizontalAlignment(Alignement alignment);
        void setVerticalAlignment(Alignement alignment);

        uint16_t getTextWidth();
        uint16_t getTextHeight();

        [[nodiscard]] bool isCursorEnabled() const;
        void setCursorEnabled(bool enable);

        [[nodiscard]] uint16_t getCursorIndex() const;
        void setCursorIndex(int16_t cursorIndex);

      private:
        struct ParseDataOutput
        {
            std::vector<std::string> m_lines;

            uint16_t m_cursorIndex; // X
            uint16_t m_cursorLine;  // Y
        };

        ParseDataOutput parse(void);
        uint16_t getUsableWidth(
            void
        ); // retourne la largeur réelle maximale utilisée par le texte (sans les bordure etc...)
        uint16_t getUsableHeight(void);

        uint16_t m_fontSize;
        color_t m_textColor;
        std::string m_text;
        Alignement m_textVerticalAlignment;
        Alignement m_textHorizontalAlignment;

        bool m_hasCursor;
        int16_t m_cursorIndex;
    };
} // namespace gui::elements

#endif // LABEL_HPP
