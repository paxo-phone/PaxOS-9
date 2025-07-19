#ifndef INPUT_HPP
#define INPUT_HPP

#include "../ElementBase.hpp"
#include "Image.hpp"
#include "Label.hpp"

#define INPUT_WIDTH 250
#define INPUT_HEIGHT 46

namespace gui::elements
{
    /**
     * @brief Input is a label where you can write text by clicking on it (open a keyboard)
     **/
    class Input final : public ElementBase
    {
      public:
        Input(uint16_t x, uint16_t y);
        ~Input();

        enum Alignement
        {
            CENTER,
            LEFT,
            RIGHT,
            UP,
            DOWN
        };

        void render();

        void widgetUpdate();

        void setText(const std::string& text);
        void setPlaceHolder(const std::string& text);
        void setTitle(const std::string& text);

        std::string getText();
        std::string getPlaceHolder();
        std::string getTitle();

      private:
        Label* m_text = nullptr;
        Label* m_title = nullptr;
        Image* m_clear = nullptr;
        bool m_hasText = false;

        std::string m_placeHolder = "";
    };
} // namespace gui::elements

#endif // INPUT_HPP
