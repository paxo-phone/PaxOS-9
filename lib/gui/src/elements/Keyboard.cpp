//
// Created by Charles on 13/03/2024.
//

#include "Keyboard.hpp"

#include <iostream>
#include <graphics.hpp>
#include <Surface.hpp>

// 0x0_ => Control chars
constexpr char KEY_NULL = 0x00;
constexpr char KEY_EXIT = 0x01;

// 0x1_ => Special Chars
constexpr char KEY_SPACE = 0x10;
constexpr char KEY_BACKSPACE = 0x11;

// 0x2_ => Modifiers
constexpr char KEY_CAPS = 0x20;

// -0x1_ => Keyboard Layouts
constexpr char KEY_LAYOUT_STANDARD = -0x10; // Lowercase or Uppercase, based on context
constexpr char KEY_LAYOUT_LOWERCASE = -0x11;
constexpr char KEY_LAYOUT_UPPERCASE = -0x12;
constexpr char KEY_LAYOUT_NUMBERS = -0x13;

// Caps
constexpr uint8_t CAPS_NONE = 0;
constexpr uint8_t CAPS_ONCE = 1;
constexpr uint8_t CAPS_LOCK = 2;

namespace gui::elements {
    Keyboard::Keyboard()
    {
        m_width = graphics::getScreenWidth();
        m_height = graphics::getScreenHeight();

        m_x = 0;
        m_y = 0;

        m_backgroundColor = graphics::packRGB565(255, 255, 255);

        m_hasEvents = true;

        m_caps = CAPS_NONE;

        auto thisPathIsUselessButGraphicsLibIsStillBroken = storage::Path("system/keyboard/caps_icon_0.png");
        m_capsIcon0 = std::make_unique<graphics::SImage>(thisPathIsUselessButGraphicsLibIsStillBroken);

        thisPathIsUselessButGraphicsLibIsStillBroken = storage::Path("system/keyboard/caps_icon_1.png");
        m_capsIcon1 = std::make_unique<graphics::SImage>(thisPathIsUselessButGraphicsLibIsStillBroken);

        thisPathIsUselessButGraphicsLibIsStillBroken = storage::Path("system/keyboard/caps_icon_2.png");
        m_capsIcon2 = std::make_unique<graphics::SImage>(thisPathIsUselessButGraphicsLibIsStillBroken);

        thisPathIsUselessButGraphicsLibIsStillBroken = storage::Path("system/keyboard/backspace_icon.png");
        m_backspaceIcon = std::make_unique<graphics::SImage>(thisPathIsUselessButGraphicsLibIsStillBroken);

        m_layoutLowercase = new char*[4];
        m_layoutLowercase[0] = new char[10]{'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'};
        m_layoutLowercase[1] = new char[10]{'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm'};
        m_layoutLowercase[2] = new char[9]{KEY_CAPS, 'w', 'x', 'c', 'v', 'b', 'n', '\'', KEY_BACKSPACE};
        m_layoutLowercase[3] = new char[3]{KEY_LAYOUT_NUMBERS, KEY_SPACE, KEY_EXIT};

        m_layoutUppercase = new char*[4];
        m_layoutUppercase[0] = new char[10]{'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'};
        m_layoutUppercase[1] = new char[10]{'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M'};
        m_layoutUppercase[2] = new char[9]{KEY_CAPS, 'W', 'X', 'C', 'V', 'B', 'N', '\'', KEY_BACKSPACE};
        m_layoutUppercase[3] = new char[3]{KEY_LAYOUT_NUMBERS, KEY_SPACE, KEY_EXIT};

        m_layoutNumbers = new char*[4];
        m_layoutNumbers[0] = new char[10]{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
        m_layoutNumbers[1] = new char[10]{'+', '-', '*', '/', '(', ')', '[', ']', '<', '>'};
        m_layoutNumbers[2] = new char[9]{KEY_CAPS, '_', ',', '.', ':', ';', '!', '?', KEY_BACKSPACE};
        m_layoutNumbers[3] = new char[3]{KEY_LAYOUT_STANDARD, KEY_SPACE, KEY_EXIT};

        m_currentLayout = m_layoutLowercase;
    }

    Keyboard::~Keyboard() = default;

    void Keyboard::render()
    {
        m_surface->fillRect(0, 0, m_width, m_height, m_backgroundColor);

        // Input box
        drawInputBox();

        // Keyboard box
        // m_surface->fillRect(30, 140, 420, 160, graphics::packRGB565(255, 255, 255));

        // Draw keys
        drawKeys();
    }

    void Keyboard::onReleased()
    {
        // Get touch position
        int16_t touchX, touchY;
        getLastTouchPosRel(&touchX, &touchY);

        // std::cout << touchX << ", " << touchY << std::endl;

        // m_surface->drawRoundRect(
        //     static_cast<int16_t>(touchX - 5),
        //     static_cast<int16_t>(touchY - 5),
        //     10,
        //     10,
        //     10,
        //     color
        // );

        const char pressedKey = getKey(touchX, touchY);
        if (pressedKey == KEY_NULL)
        {
            return;
        }

        processKey(pressedKey);
    }

    std::string Keyboard::getText()
    {
        const std::string output = m_buffer;

        m_buffer = "";

        return output;
    }

    void Keyboard::drawKeys()
    {
        // Draw every keys
        drawKeyRow(140, 10, m_currentLayout[0]);
        drawKeyRow(180, 10, m_currentLayout[1]);
        drawKeyRow(220, 9, m_currentLayout[2]);
        drawLastRow(30, 260);
    }

    void Keyboard::drawKeyRow(const int16_t y, const uint8_t count, char* keys)
    {
        const float keyWidth = 420.0f / static_cast<float>(count);

        for (uint16_t i = 0; i < count; i++)
        {
            drawKey(
                static_cast<int16_t>(30 + static_cast<float>(i) * keyWidth),
                y,
                static_cast<int16_t>(keyWidth),
                keys[i]
            );
        }
    }

    void Keyboard::drawKey(const int16_t x, const int16_t y, const uint16_t w, char key)
    {
        if (key == KEY_CAPS)
        {
            if (m_caps == CAPS_NONE)
            {
                m_surface->drawImage(*m_capsIcon0, static_cast<int16_t>(x + 3), y);
            }
            else if (m_caps == CAPS_ONCE)
            {
                m_surface->drawImage(*m_capsIcon1, static_cast<int16_t>(x + 3), y);
            }
            else
            {
                m_surface->drawImage(*m_capsIcon2, static_cast<int16_t>(x + 3), y);
            }

            return;
        }
        else if (key == KEY_BACKSPACE)
        {
            m_surface->drawImage(*m_backspaceIcon, static_cast<int16_t>(x + 3), y);
        }

        std::string keyString = std::string(1, key);

        // m_surface->drawRect(x, y, w, 40, graphics::packRGB565(0, 0, 0));
        m_surface->drawTextCentered(keyString, x, static_cast<int16_t>(y + 4), w);
    }

    void Keyboard::drawLastRow(const int16_t x, const int16_t y)
    {
        // m_surface->drawRect(x, y, 50, 40, graphics::packRGB565(0, 0, 0));
        // m_surface->drawRect(static_cast<int16_t>(x + 50), y, 320, 40, graphics::packRGB565(0, 0, 0));
        // m_surface->drawRect(static_cast<int16_t>(x + 370), y, 50, 40, graphics::packRGB565(0, 0, 0));

        std::string thisStringIsUselessButNecessaryBecauseGraphicsLibIsBrokenAsOfNow;

        if (m_currentLayout == m_layoutNumbers)
        {
            thisStringIsUselessButNecessaryBecauseGraphicsLibIsBrokenAsOfNow = "ABC";
        }
        else
        {
            thisStringIsUselessButNecessaryBecauseGraphicsLibIsBrokenAsOfNow = "123";
        }
        m_surface->drawTextCentered(
            thisStringIsUselessButNecessaryBecauseGraphicsLibIsBrokenAsOfNow,
            static_cast<int16_t>(x),
            static_cast<int16_t>(y + 5),
            50
        );

        m_surface->fillRect(
            static_cast<int16_t>(x + 100),
            static_cast<int16_t>(y + 30),
            220,
            2,
            graphics::packRGB565(0, 0, 0)
        );

        thisStringIsUselessButNecessaryBecauseGraphicsLibIsBrokenAsOfNow = "retour";
        m_surface->drawTextCentered(
            thisStringIsUselessButNecessaryBecauseGraphicsLibIsBrokenAsOfNow,
            static_cast<int16_t>(x + 350),
            static_cast<int16_t>(y + 5),
            50
        );
    }

    char Keyboard::getKey(const int16_t x, const int16_t y)
    {
        // Check if the position is in the keyboard box
        if (!(x >= 30 && x <= 450 && y >= 140 && y <= 300))
        {
            return KEY_NULL;
        }

        uint8_t row;
        uint8_t column;

        // Get the row
        if (y <= 180)
        {
            // First Row
            row = 0;

            // Get column
            column = getKeyCol(x, 10);
        }
        else if (y <= 220)
        {
            // Second Row
            row = 1;

            // Get column
            column = getKeyCol(x, 10);
        }
        else if (y <= 260)
        {
            // Third Row
            row = 2;

            // Get column
            column = getKeyCol(x, 9);
        }
        else
        {
            // Last Row
            row = 3;

            // Get column
            if (x <= 80)
            {
                column = 0;
            }
            else if (x <= 370)
            {
                column = 1;
            } else
            {
                column = 2;
            }
        }

        return m_currentLayout[row][column];
    }

    uint8_t Keyboard::getKeyCol(const int16_t x, const uint8_t keyCount)
    {
        float boxX = 30;
        const float keyWidth = 420.0f / static_cast<float>(keyCount);

        for (uint8_t i = 0; i < keyCount; i++)
        {
            if (static_cast<float>(x) >= boxX && static_cast<float>(x) <= boxX + keyWidth)
            {
                return i;
            }

            boxX += keyWidth;
        }

        return -1;
    }

    /**
     * Execute the needed action for the key
     * @param key The key to process
     */
    void Keyboard::processKey(const char key)
    {
        switch (key)
        {
        case KEY_NULL:
            return;
        case KEY_EXIT:
            std::cout << "KEYBOARD EXIT" << std::endl;

            return;
        case KEY_SPACE:
            m_buffer += " ";
            break;
        case KEY_BACKSPACE:
            if (!m_buffer.empty())
            {
                m_buffer.pop_back();
            }
            break;
        case KEY_CAPS:
            if (m_caps == CAPS_NONE)
            {
                m_currentLayout = m_layoutUppercase;
                m_caps = CAPS_ONCE;

                markDirty();
            }
            else if (m_caps == CAPS_ONCE)
            {
                m_currentLayout = m_layoutUppercase;
                m_caps = CAPS_LOCK;

                markDirty();
            } else if (m_caps == CAPS_LOCK)
            {
                m_currentLayout = m_layoutLowercase;
                m_caps = CAPS_NONE;

                markDirty();
            }
            return; // QUIT
        case KEY_LAYOUT_NUMBERS:
            m_currentLayout = m_layoutNumbers;
            markDirty();

            return;
        case KEY_LAYOUT_STANDARD:
            if (m_caps == CAPS_NONE)
            {
                m_currentLayout = m_layoutLowercase;
                markDirty();
            }
            else
            {
                m_currentLayout = m_layoutUppercase;
                markDirty();
            }

            return;
        default:
            m_buffer += std::string(1, key);

            // Disable caps if not locked
            if (m_caps == CAPS_ONCE)
            {
                m_currentLayout = m_layoutLowercase;
                m_caps = CAPS_NONE;
            }

            break;
        }

        // Redraw input box
        // drawInputBox(); <= Useless, because "markDirty" redraws it

        // Mark dirty
        markDirty();
    }

    void Keyboard::drawInputBox()
    {
        // m_surface->fillRect(30, 30, 420, 100, graphics::packRGB565(255, 255, 255));

        // Draw text
        m_surface->setFont(graphics::ARIAL);
        m_surface->setFontSize(24);
        m_surface->setTextColor(graphics::packRGB565(0, 0, 0));
        m_surface->drawText(m_buffer, 30, 30);
    }

    void Keyboard::markDirty()
    {
        m_isDrawn = false;
        m_isRendered = false;
    }
} // gui::elements