//
// Created by Charles on 13/03/2024.
//

#include "Keyboard.hpp"

#include <iostream>
#include <graphics.hpp>
#include <Surface.hpp>

constexpr char KEY_NULL = 0x00;
constexpr char KEY_EXIT = 0x01;

// 0x1_ => Special Chars
constexpr char KEY_SPACE = 0x10;
constexpr char KEY_BACKSPACE = 0x11;

// 0x2_ => Modifiers
constexpr char KEY_CAPS = 0x20;

// 0x3_ => Keyboard Categories
constexpr char KEY_CATEGORY_NUMBERS = 0x30;

namespace gui::elements {
    Keyboard::Keyboard()
    {
        m_width = graphics::getScreenWidth();
        m_height = graphics::getScreenHeight();

        m_x = 0;
        m_y = 0;

        m_hasEvents = true;

        keys = new char*[4];
        keys[0] = new char[]{'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'};
        keys[1] = new char[]{'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm'};
        keys[2] = new char[]{KEY_CAPS, 'w', 'x', 'c', 'v', 'b', 'n', '\'', KEY_BACKSPACE};
        keys[3] = new char[]{KEY_CATEGORY_NUMBERS, KEY_SPACE, KEY_EXIT};
    }

    Keyboard::~Keyboard() = default;

    void Keyboard::render()
    {
        // Input box
        drawInputBox();

        // Keyboard box
        m_surface->fillRect(30, 140, 420, 160, graphics::packRGB565(255, 255, 255));

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

        // Mark dirty
        // m_isDrawn = false;

        const char pressedKey = getKey(touchX, touchY);
        if (pressedKey == KEY_NULL)
        {
            return;
        }

        processKey(pressedKey);
    }

    void Keyboard::drawKeys()
    {
        drawKeyRow(140, 10, keys[0]);
        drawKeyRow(180, 10, keys[1]);
        drawKeyRow(220, 9, keys[2]);
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
        std::string keyString = std::string(1, key);

        m_surface->drawRect(x, y, w, 40, graphics::packRGB565(0, 0, 0));
        m_surface->drawTextCentered(keyString, x, static_cast<int16_t>(y + 4), w);
    }

    void Keyboard::drawLastRow(const int16_t x, const int16_t y)
    {
        m_surface->drawRect(x, y, 50, 40, graphics::packRGB565(0, 0, 0));
        m_surface->drawRect(static_cast<int16_t>(x + 50), y, 320, 40, graphics::packRGB565(0, 0, 0));
        m_surface->drawRect(static_cast<int16_t>(x + 370), y, 50, 40, graphics::packRGB565(0, 0, 0));

        m_surface->fillRect(
            static_cast<int16_t>(x + 100),
            static_cast<int16_t>(y + 30),
            220,
            2,
            graphics::packRGB565(0, 0, 0)
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

        return keys[row][column];
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
            break;
        case KEY_EXIT:
            break;
        case KEY_SPACE:
            buffer += " ";
            break;
        case KEY_BACKSPACE:
            break;
        case KEY_CAPS:
            break;
        case KEY_CATEGORY_NUMBERS:
            break;
        default:
            buffer += std::string(1, key);
            break;
        }

        // Redraw input box
        drawInputBox();

        // Mark dirty
        m_isDrawn = false;
    }

    void Keyboard::drawInputBox()
    {
        m_surface->fillRect(30, 30, 420, 100, graphics::packRGB565(255, 255, 255));

        // Draw text
        m_surface->setFont(graphics::ARIAL);
        m_surface->setFontSize(24);
        m_surface->setTextColor(graphics::packRGB565(0, 0, 0));
        m_surface->drawText(buffer, 30, 30);
    }
} // gui::elements