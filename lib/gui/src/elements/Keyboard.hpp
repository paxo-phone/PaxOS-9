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

        std::shared_ptr<Label> m_label;

        bool m_exit = false;

        uint8_t m_currentLayout;

        char** m_layoutLowercase;
        char** m_layoutUppercase;
        char** m_layoutNumbers;

        uint8_t m_caps;

        std::shared_ptr<Canvas> m_keysCanvas;

        std::shared_ptr<Image> m_capsIcon0;
        std::shared_ptr<Image> m_capsIcon1;
        std::shared_ptr<Image> m_capsIcon2;
        std::shared_ptr<Image> m_backspaceIcon;
        std::shared_ptr<Image> m_layoutIcon0;
        std::shared_ptr<Image> m_layoutIcon1;
        std::shared_ptr<Image> m_exitIcon;
        std::shared_ptr<Image> m_confirmIcon;
        std::shared_ptr<Image> m_trackpadActiveIcon;

        std::shared_ptr<Box> m_capsBox;
        std::shared_ptr<Box> m_layoutBox;
        std::shared_ptr<Box> m_backspaceBox;
        std::shared_ptr<Box> m_exitBox;
        std::shared_ptr<Box> m_confirmBox;
        std::shared_ptr<Box> m_trackpadActiveBox;

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
