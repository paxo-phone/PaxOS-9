//
// Created by Charles on 13/03/2024.
//

#ifndef KEYBOARD2_HPP
#define KEYBOARD2_HPP

#include "../ElementBase.hpp"
#include "Box.hpp"
#include "Canvas.hpp"
#include "Filter.hpp"
#include "Image.hpp"
#include "Label.hpp"

namespace gui::elements {
    class Keyboard2 final : public ElementBase {
    public:

        struct Coord {
            int16_t row;
            int16_t column;
        };

        explicit Keyboard2(const std::string &defaultText = "");

        ~Keyboard2() override;

        void render() override;

        void widgetUpdate() override;

        /**
         * Returns the content of the Keyboard2's input AND CLEARS IT.
         * @return the content of the Keyboard2's input
         */
        std::string getText();

        /**
         * @deprecated Please use "hasExitKeyBeenPressed()"
         */
        [[nodiscard]] bool quitting() const { return m_exit; }

        [[nodiscard]] bool hasExitKeyBeenPressed() const;

        void setPlaceholder(const std::string &placeholder);

    private:
        std::string m_buffer;
        std::string m_placeholder;
        std::string m_defaultText;

        Label *m_label;

        bool m_exit = false;

        uint8_t m_currentLayout;

        char **m_layoutLowercase;
        char **m_layoutUppercase;
        char **m_layoutNumbers;

        uint8_t m_caps;

        Canvas *m_keysCanvas;

        Image *m_capsIcon0;
        Image *m_capsIcon1;
        Image *m_capsIcon2;
        Image *m_backspaceIcon;
        Image *m_layoutIcon0;
        Image *m_layoutIcon1;
        Image *m_exitIcon;
        Image *m_confirmIcon;
        Image* m_trackpadActiveIcon;

        Box *m_capsBox;
        Box *m_layoutBox;
        Box *m_backspaceBox;
        Box *m_exitBox;
        Box *m_confirmBox;
        Box* m_trackpadActiveBox;

        Box* m_boxdKeyPressed_Left10;
        Canvas *m_CanevasKeyPressed_Left10;
        Box* m_boxdKeyPressed_Center10;
        Canvas *m_CanevasKeyPressed_Center10;
        Box* m_boxdKeyPressed_Right10;
        Canvas *m_CanevasKeyPressed_Right10;
        Box* m_boxdKeyPressed_Center9;
        Canvas *m_CanevasKeyPressed_Center9;

        uint8_t m_trackpadTicks;
        int32_t m_trackpadLastDeltaX;

        int32_t boxHeight;
        int32_t hauteurKeyboard;
        int32_t offsetBord;
        float keyZoomRatio;
        uint8_t hauteurRetourZoom;


        uint8_t m_zoomKeyTicks;
        Keyboard2::Coord oldCoord;
        void drawZoomKeyLeft10 (uint16_t x, uint16_t y,uint16_t keyWidth , char key);
        void drawZoomKeyCenter10 (uint16_t x, uint16_t y,uint16_t keyWidth , char key);
        void drawZoomKeyRight10 (uint16_t x, uint16_t y,uint16_t keyWidth , char key);
        void drawZoomKeyCenter9 (uint16_t x, uint16_t y,uint16_t keyWidth , char key);
        Keyboard2::Coord getRowColumn(const int16_t x, const int16_t y) const;
        void zoomKeyUpdate();
        bool isZoomdActive() const;
        void enableKeyZoom(uint16_t row, uint16_t column, char pressedKey);
        void disableKeyZoom(uint16_t row, uint16_t column);

        void drawKeys() const;

        void drawKeyRow(int16_t y, const char *keys) const;

        void drawKey(int16_t x, int16_t y, uint16_t w, char key) const;

        void drawLastRow(const int16_t y) const;

        [[nodiscard]] char getKey(int16_t x, int16_t y) const;

        [[nodiscard]] uint8_t getKeyCol(const int16_t x, const uint8_t keyCount) const;


        void processKey(char key);

        void drawInputBox() const;

        void updateCapsIcon() const;

        void updateLayoutIcon() const;

        [[nodiscard]] char **getLayoutCharMap() const;

        void trackpadUpdate();

        [[nodiscard]] bool isPointInTrackpad(int16_t x, int16_t y) const;

        [[nodiscard]] bool isTrackpadActive() const;

        void addChar(char value);

        void removeChar();

    };
} // gui::elements

#endif //Keyboard2_HPP
