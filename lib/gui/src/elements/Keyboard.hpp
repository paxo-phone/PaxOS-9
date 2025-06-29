//
// Created by Charles on 13/03/2024.
//

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../ElementBase.hpp"
#include "Box.hpp"
#include "Canvas.hpp"
#include "Filter.hpp"
#include "Image.hpp"
#include "Label.hpp"

#define OUTPUT_VOCAB_SIZE 27

namespace gui::elements
{
    class Keyboard final : public ElementBase
    {
      public:
        explicit Keyboard(const std::string& defaultText = "");

        ~Keyboard() override;

        void render() override;

        void widgetUpdate() override;

        /**
         * Returns the content of the keyboard's input AND CLEARS IT.
         * @return the content of the keyboard's input
         */
        std::string getText();

        /**
         * @deprecated Please use "hasExitKeyBeenPressed()"
         */
        [[nodiscard]] bool quitting() const
        {
            return m_exit;
        }

        [[nodiscard]] bool hasExitKeyBeenPressed() const;

        void setPlaceholder(const std::string& placeholder);

      private:
        std::string m_buffer;
        std::string m_placeholder;
        std::string m_defaultText;

        Label* m_label;

        bool m_exit = false;

        uint8_t m_currentLayout;

        char** m_layoutLowercase;
        char** m_layoutUppercase;
        char** m_layoutNumbers;

        uint8_t m_caps;

        Canvas* m_keysCanvas;

        Image* m_capsIcon0;
        Image* m_capsIcon1;
        Image* m_capsIcon2;
        Image* m_backspaceIcon;
        Image* m_layoutIcon0;
        Image* m_layoutIcon1;
        Image* m_exitIcon;
        Image* m_confirmIcon;
        Image* m_trackpadActiveIcon;

        Box* m_capsBox;
        Box* m_layoutBox;
        Box* m_backspaceBox;
        Box* m_exitBox;
        Box* m_confirmBox;
        Box* m_trackpadActiveBox;

        uint8_t m_trackpadTicks;
        int32_t m_trackpadLastDeltaX;

        float probabilities[OUTPUT_VOCAB_SIZE] = {0.0f};

        void drawKeys() const;

        void drawKeyRow(int16_t y, uint8_t count, const char* keys) const;

        void drawKey(int16_t x, int16_t y, uint16_t w, char key) const;

        void drawLastRow() const;

        [[nodiscard]] char getKey(int16_t x, int16_t y) const;

        static uint8_t getKeyCol(int16_t x, uint8_t keyCount);

        void processKey(char key);

        void drawInputBox() const;

        void updateCapsIcon() const;

        void updateLayoutIcon() const;

        [[nodiscard]] char** getLayoutCharMap() const;

        void trackpadUpdate();

        [[nodiscard]] bool isPointInTrackpad(int16_t x, int16_t y) const;

        [[nodiscard]] bool isTrackpadActive() const;

        void addChar(char value);

        void removeChar();
    };
} // namespace gui::elements

#endif // KEYBOARD_HPP
