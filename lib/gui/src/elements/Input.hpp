#ifndef INPUT_HPP
#define INPUT_HPP

#include "../ElementBase.hpp"

#include "Label.hpp"
#include "Image.hpp"

namespace gui::elements
{
    class Input final : public ElementBase
    {
    public:        
        Input(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~Input() override;

        enum Alignement {
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
} // gui::elements

#endif //INPUT_HPP
