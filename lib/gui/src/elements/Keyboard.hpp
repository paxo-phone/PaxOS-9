//
// Created by Charles on 13/03/2024.
//

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../ElementBase.hpp"

namespace gui::elements
{
    class Keyboard final : public ElementBase
    {
    public:
        Keyboard();
        ~Keyboard() override;

        void render() override;
        void onReleased() override;

        void drawKeys();
        void drawKeyRow(int16_t y, uint8_t count, char* keys);
        void drawKey(int16_t x, int16_t y, uint16_t w, char key);

        void drawLastRow(int16_t x, int16_t y);

        char getKey(int16_t x, int16_t y);
        uint8_t getKeyCol(int16_t x, uint8_t keyCount);

        void processKey(char key);

        void drawInputBox();

    private:
        std::string buffer;

        char **keys;
    };
} // gui::elements

#endif //KEYBOARD_HPP
