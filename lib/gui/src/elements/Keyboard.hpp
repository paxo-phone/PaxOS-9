//
// Created by Charles on 13/03/2024.
//

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../ElementBase.hpp"
#include "Box.hpp"
#include "Image.hpp"

namespace gui::elements
{
    class Keyboard final : public ElementBase
    {
    public:
        Keyboard();
        ~Keyboard() override;

        void render() override;
        void onReleased() override;

        /**
         * Returns the content of the keyboard's input AND CLEARS IT.
         * @return the content of the keyboard's input
         */
        std::string getText();

        bool quitting() {return quit;}

    private:
        std::string m_buffer;
        bool quit = false;

        char **m_currentLayout;

        char **m_layoutLowercase;
        char **m_layoutUppercase;
        char **m_layoutNumbers;

        uint8_t m_caps;

        Image* m_capsIcon0;
        Image* m_capsIcon1;
        Image* m_capsIcon2;

        Image* m_backspaceIcon;

        Box* m_capsBox;
        Box* m_layoutBox;
        Box* m_backspaceBox;
        Box* m_exitBox;

        void drawKeys();
        void drawKeyRow(int16_t y, uint8_t count, char* keys);
        void drawKey(int16_t x, int16_t y, uint16_t w, char key);

        void drawLastRow(int16_t x, int16_t y);

        char getKey(int16_t x, int16_t y);
        uint8_t getKeyCol(int16_t x, uint8_t keyCount);

        void processKey(char key);

        void drawInputBox();

        void markDirty();
    };
} // gui::elements

#endif //KEYBOARD_HPP
