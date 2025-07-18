//
// Created by Charles on 13/03/2024.
//

#include "Keyboard.hpp"

#include "Box.hpp"
#include "Filter.hpp"
#include "Image.hpp"

#include <Surface.hpp>
#include <dynamic_hitboxes.hpp>
#include <graphics.hpp>
#include <iostream>
#include <libsystem.hpp>

// Layouts
constexpr char LAYOUT_LOWERCASE = 0;
constexpr char LAYOUT_UPPERCASE = 1;
constexpr char LAYOUT_NUMBERS = 2;

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

namespace gui::elements
{
    Keyboard::Keyboard(const std::string& defaultText)
    {
        m_buffer = defaultText;
        m_defaultText = defaultText;

        if (graphics::getScreenOrientation() == graphics::LANDSCAPE)
        {
            width_ = graphics::getScreenWidth();
            height_ = graphics::getScreenHeight();
        }
        else
        {
            std::cerr << "[Warning] It seems that you are using the Keyboard "
                         "element in potrait mode."
                      << std::endl;

            width_ = graphics::getScreenHeight();
            height_ = graphics::getScreenWidth();
        }

        x_ = 0;
        y_ = 0;

        backgroundColor_ = COLOR_WHITE;

        hasEvents_ = true;

        m_caps = CAPS_NONE;

        m_keysCanvas = std::make_shared<Canvas>(30, 140, 420, 160);
        addChild(m_keysCanvas);

        m_layoutLowercase = new char*[4];
        m_layoutLowercase[0] = new char[10]{'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'};
        m_layoutLowercase[1] = new char[10]{'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm'};
        m_layoutLowercase[2] =
            new char[9]{KEY_CAPS, 'w', 'x', 'c', 'v', 'b', 'n', '\'', KEY_BACKSPACE};
        m_layoutLowercase[3] = new char[3]{KEY_LAYOUT_NUMBERS, KEY_SPACE, KEY_EXIT};

        m_layoutUppercase = new char*[4];
        m_layoutUppercase[0] = new char[10]{'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'};
        m_layoutUppercase[1] = new char[10]{'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M'};
        m_layoutUppercase[2] =
            new char[9]{KEY_CAPS, 'W', 'X', 'C', 'V', 'B', 'N', '\'', KEY_BACKSPACE};
        m_layoutUppercase[3] = new char[3]{KEY_LAYOUT_NUMBERS, KEY_SPACE, KEY_EXIT};

        m_layoutNumbers = new char*[4];
        m_layoutNumbers[0] = new char[10]{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
        m_layoutNumbers[1] = new char[10]{'+', '-', '*', '/', '(', ')', '[', ']', '<', '>'};
        m_layoutNumbers[2] =
            new char[9]{KEY_CAPS, '_', ',', '.', ':', ';', '!', '?', KEY_BACKSPACE};
        m_layoutNumbers[3] = new char[3]{KEY_LAYOUT_STANDARD, KEY_SPACE, KEY_EXIT};

        m_currentLayout = LAYOUT_LOWERCASE;

        // Create label for text
        m_label = std::make_shared<Label>(30, 30, 380, 80);
        // m_label->setFont(graphics::ARIAL);
        m_label->setFontSize(24);
        m_label->setCursorEnabled(true);
        m_label->setText(m_buffer);
        m_label->setCursorIndex(static_cast<int16_t>(m_buffer.length()));
        addChild(m_label);

        // Create images box (for better performances ?)
        m_capsBox = std::make_shared<Box>(30, 220, 47, 40);
        m_layoutBox = std::make_shared<Box>(30, 260, 80, 40);
        m_backspaceBox = std::make_shared<Box>(403, 220, 47, 40);
        m_exitBox = std::make_shared<Box>(370, 260, 80, 40);
        m_confirmBox = std::make_shared<Box>(410, 30, 40, 40);

        // Create images
        m_capsIcon0 =
            std::make_shared<Image>(storage::Path("system/keyboard/caps_icon_0.png"), 0, 0, 40, 40);
        m_capsIcon1 =
            std::make_shared<Image>(storage::Path("system/keyboard/caps_icon_1.png"), 0, 0, 40, 40);
        m_capsIcon2 =
            std::make_shared<Image>(storage::Path("system/keyboard/caps_icon_2.png"), 0, 0, 40, 40);
        m_backspaceIcon = std::make_shared<Image>(
            storage::Path("system/keyboard/backspace_icon.png"),
            0,
            0,
            40,
            40
        );
        m_layoutIcon0 = std::make_shared<Image>(
            storage::Path("system/keyboard/layout_icon_0.png"),
            20,
            0,
            40,
            40
        );
        m_layoutIcon1 = std::make_shared<Image>(
            storage::Path("system/keyboard/layout_icon_1.png"),
            20,
            0,
            40,
            40
        );
        m_exitIcon =
            std::make_shared<Image>(storage::Path("system/keyboard/exit_icon.png"), 0, 0, 40, 40);
        m_confirmIcon = std::make_shared<Image>(
            storage::Path("system/keyboard/confirm_icon.png"),
            0,
            0,
            40,
            40
        );

        // Load images into RAM
        m_capsIcon0->load();
        m_capsIcon1->load();
        m_capsIcon2->load();
        m_backspaceIcon->load();
        m_layoutIcon0->load();

        m_layoutIcon1->load();
        m_exitIcon->load();

        // m_confirmIcon->load();

        // Add images to boxes
        m_capsBox->addChild(m_capsIcon0);
        m_capsBox->addChild(m_capsIcon1);
        m_capsBox->addChild(m_capsIcon2);
        m_backspaceBox->addChild(m_backspaceIcon);
        m_layoutBox->addChild(m_layoutIcon0);
        m_layoutBox->addChild(m_layoutIcon1);
        m_exitBox->addChild(m_exitIcon);
        m_confirmBox->addChild(m_confirmIcon);

        // Add boxes
        addChild(m_capsBox);
        addChild(m_layoutBox);
        addChild(m_backspaceBox);
        addChild(m_exitBox);
        addChild(m_confirmBox);

        updateCapsIcon();
        updateLayoutIcon();

        m_trackpadActiveBox = std::make_shared<Box>(192, 112, 96, 96);
        m_trackpadActiveBox->setBackgroundColor(TFT_BLACK);
        m_trackpadActiveBox->setRadius(8);
        addChild(m_trackpadActiveBox);

        m_trackpadActiveIcon = std::make_shared<Image>(
            storage::Path("system/keyboard/trackpad_active_icon.png"),
            16,
            16,
            64,
            64
        );
        m_trackpadActiveIcon->load(TFT_BLACK);
        m_trackpadActiveBox->addChild(m_trackpadActiveIcon);

        m_trackpadActiveBox->disable();

        m_trackpadTicks = 0;
        m_trackpadLastDeltaX = 0;
    }

    Keyboard::~Keyboard() = default;

    void Keyboard::render()
    {
        surface_->fillRect(0, 0, width_, height_, backgroundColor_);

        // Input box
        drawInputBox();

        if (!isTrackpadActive())
        {
            // Draw keys
            drawKeys();
        }
    }

    void Keyboard::widgetUpdate()
    {
        if (isTouched())
        {
            // Get touch position
            int16_t touchX, touchY;
            getLastTouchPosRel(&touchX, &touchY);

            const char pressedKey = getKey(touchX, touchY);
            if (pressedKey == KEY_NULL)
                return;

            processKey(pressedKey);
        }

        if (m_exitBox->isTouched())
        {
            m_buffer = m_defaultText; // Reset text
            m_exit = true;
        }

        if (m_confirmBox->isTouched())
            m_exit = true;

        if (m_backspaceBox->isTouched())
        {
            removeChar();

            // Redraw input box
            drawInputBox();
        }

        if (m_capsBox->isTouched())
        {
            switch (m_caps)
            {
            case CAPS_NONE:
            default:
                m_currentLayout = LAYOUT_UPPERCASE;
                m_caps = CAPS_ONCE;
                break;
            case CAPS_ONCE:
                m_currentLayout = LAYOUT_UPPERCASE;
                m_caps = CAPS_LOCK;
                break;
            case CAPS_LOCK:
                m_currentLayout = LAYOUT_LOWERCASE;
                m_caps = CAPS_NONE;
                break;
            }

            updateCapsIcon();
        }

        if (m_layoutBox->isTouched())
        {
            if (m_currentLayout == LAYOUT_LOWERCASE || m_currentLayout == LAYOUT_UPPERCASE)
            {
                m_currentLayout = LAYOUT_NUMBERS;
            }
            else if (m_currentLayout == LAYOUT_NUMBERS)
            {
                if (m_caps == CAPS_NONE)
                    m_currentLayout = LAYOUT_LOWERCASE;
                else
                    m_currentLayout = LAYOUT_UPPERCASE;
            }

            drawKeys();
            updateLayoutIcon();
        }

        trackpadUpdate();
    }

    std::string Keyboard::getText()
    {
        const std::string output = m_buffer;

        m_buffer = "";

        return output;
    }

    void Keyboard::drawKeys() const
    {
        // Reset default settings
        m_keysCanvas
            ->fillRect(0, 0, m_keysCanvas->getWidth(), m_keysCanvas->getHeight(), COLOR_WHITE);

        // Draw every keys
        drawKeyRow(0, 10, getLayoutCharMap()[0]);
        drawKeyRow(40, 10, getLayoutCharMap()[1]);
        drawKeyRow(80, 9, getLayoutCharMap()[2]);
        drawLastRow();
    }

    void Keyboard::drawKeyRow(const int16_t y, const uint8_t count, const char* keys) const
    {
        const float keyWidth = 420.0f / static_cast<float>(count);

        for (uint16_t i = 0; i < count; i++)
        {
            drawKey(
                static_cast<int16_t>(static_cast<float>(i) * keyWidth),
                y,
                static_cast<int16_t>(keyWidth),
                keys[i]
            );
        }
    }

    void Keyboard::drawKey(const int16_t x, const int16_t y, const uint16_t w, const char key) const
    {
        auto keyString = std::string(1, key);

        m_keysCanvas->drawTextCenteredInRect(
            x,
            y,
            w,
            40,
            keyString,
            graphics::packRGB565(0, 0, 0),
            true,
            true,
            32
        );
    }

    void Keyboard::drawLastRow() const
    {
        // Draw spacebar
        m_keysCanvas->fillRect(100, 150, 220, 2, graphics::packRGB565(0, 0, 0));
    }

    char Keyboard::getKey(const int16_t x, const int16_t y) const
    {
        // Check if the position is in the keyboard box
        if (!(x >= 30 && x <= 450 && y >= 140 && y <= 300))
            return KEY_NULL;

        int bestRow = -1;
        int bestColumn = -1;
        float bestDistance = 1e9f;

        // Define the number of keys per row and the vertical centers for each row
        const int keysPerRow[4] = {10, 10, 9, 3};
        const int rowCenterY[4] =
            {160, 200, 240, 280}; // Centers of rows: (140+180)/2, (180+220)/2, etc.

        // Iterate over all keys
        for (int row = 0; row < 4; row++)
        {
            uint8_t keyCount = keysPerRow[row];
            float keyWidth = 420.0f / static_cast<float>(keyCount);
            for (uint8_t col = 0; col < keyCount; col++)
            {
                // Get the key
                char key = getLayoutCharMap()[row][col];
                float proba = 0.0f;

                for (int i = 0; i < OUTPUT_VOCAB_SIZE; i++)
                {
                    if (key == output_index_to_char[i])
                    {
                        proba = probabilities[i];
                        // std::cout << "Key: " << key << ", Probability: " << proba
                        // << std::endl;
                        break;
                    }
                }

                float centerX = 30.0f + (col + 0.5f) * keyWidth;
                float centerY = static_cast<float>(rowCenterY[row]);

                //$DYNAMIC_HITBOX_PLACEHOLDER$
                float dx = static_cast<float>(x) - centerX;
                float dy = static_cast<float>(y) - centerY;
                float distance = std::sqrt(dx * dx + dy * dy) - proba * 100.0f;

                if (distance < bestDistance)
                {
                    bestDistance = distance;
                    bestRow = row;
                    bestColumn = col;
                }
            }
        }

        char keyPressed = getLayoutCharMap()[bestRow][bestColumn];
        std::cout << "Key pressed: " << keyPressed << std::endl;
        return keyPressed;
    }

    uint8_t Keyboard::getKeyCol(const int16_t x, const uint8_t keyCount)
    {
        float boxX = 30;
        const float keyWidth = 420.0f / static_cast<float>(keyCount);

        for (uint8_t i = 0; i < keyCount; i++)
        {
            if (static_cast<float>(x) >= boxX && static_cast<float>(x) <= boxX + keyWidth)
                return i;

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
        case KEY_EXIT:
        case KEY_BACKSPACE:
        case KEY_CAPS:
        case KEY_LAYOUT_STANDARD:
        case KEY_LAYOUT_NUMBERS:
            // KEY_EXIT & KEY_BACKSPACE & KEY_CAPS & KEY_LAYOUT_STANDARD &
            // KEY_LAYOUT_NUMBERS are handled directly in update function
            return;
        case KEY_SPACE:
            addChar(' ');

            break;
        default:
            addChar(key);

            // Disable caps if not locked
            if (m_caps == CAPS_ONCE)
            {
                m_currentLayout = LAYOUT_LOWERCASE;
                m_caps = CAPS_NONE;

                drawKeys();
                updateCapsIcon(); // Fix bug

                return;
            }

            break;
        }

        // Redraw input box
        drawInputBox(); // <= Useless, because "markDirty" redraws it
    }

    void Keyboard::drawInputBox() const
    {
        if (m_buffer.empty())
        {
            if (m_placeholder.empty())
            {
                m_label->setText("");
                return;
            }

            // Draw placeholder
            m_label->setTextColor(graphics::packRGB565(200, 200, 200));
            m_label->setText(m_placeholder);

            m_label->setCursorEnabled(false);
        }
        else
        {
            // Draw text
            m_label->setTextColor(graphics::packRGB565(0, 0, 0));
            m_label->setText(m_buffer);

            m_label->setCursorEnabled(true);
        }

        char last1 = ' ', last2 = ' ', last3 = ' ';
        std::string alphabets;
        for (char c : m_buffer)
            if (std::isalpha(static_cast<unsigned char>(c)))
                alphabets.push_back(c);

        if (alphabets.size() >= 3)
        {
            last1 = alphabets[alphabets.size() - 3];
            last2 = alphabets[alphabets.size() - 2];
            last3 = alphabets[alphabets.size() - 1];
        }
        else if (alphabets.size() == 2)
        {
            last1 = alphabets[0];
            last2 = alphabets[1];
        }
        else if (alphabets.size() == 1)
        {
            last1 = alphabets[0];
        }

        std::cout << "Last 3 letters: " << last1 << last2 << last3 << std::endl;
        predict_next_char_probs(last1, last2, last3, const_cast<float*>(probabilities));
    }

    void Keyboard::updateCapsIcon() const
    {
        switch (m_caps)
        {
        case CAPS_NONE:
            m_capsIcon0->enable();
            m_capsIcon1->disable();
            m_capsIcon2->disable();
            break;
        case CAPS_ONCE:
            m_capsIcon0->disable();
            m_capsIcon1->enable();
            m_capsIcon2->disable();
            break;
        case CAPS_LOCK:
            m_capsIcon0->disable();
            m_capsIcon1->disable();
            m_capsIcon2->enable();
            break;
        default:;
        }
    }

    void Keyboard::updateLayoutIcon() const
    {
        switch (m_currentLayout)
        {
        case LAYOUT_LOWERCASE:
        case LAYOUT_UPPERCASE:
            m_layoutIcon0->enable();
            m_layoutIcon1->disable();
            break;
        case LAYOUT_NUMBERS:
            m_layoutIcon0->disable();
            m_layoutIcon1->enable();
            break;
        default:;
        }
    }

    bool Keyboard::hasExitKeyBeenPressed() const
    {
        return m_exit;
    }

    void Keyboard::setPlaceholder(const std::string& placeholder)
    {
        m_placeholder = placeholder;
    }

    char** Keyboard::getLayoutCharMap() const
    {
        switch (m_currentLayout)
        {
        case LAYOUT_LOWERCASE:
        default:
            return m_layoutLowercase;
        case LAYOUT_UPPERCASE:
            return m_layoutUppercase;
        case LAYOUT_NUMBERS:
            return m_layoutNumbers;
        }
    }

    void Keyboard::trackpadUpdate()
    {
        int16_t rawTouchX, rawTouchY;
        graphics::getTouchPos(&rawTouchX, &rawTouchY);

        const bool wasTrackpadActive = isTrackpadActive();

        // Check if finger is on screen
        if ((rawTouchX != -1 && rawTouchY != -1) && isPointInTrackpad(originTouchX, originTouchY))
        {
            // libsystem::log("[TRACKPAD] Raw Touch : " + std::to_string(rawTouchX)
            // + ", " + std::to_string(rawTouchY) + "."); libsystem::log("[TRACKPAD]
            // Last Touch : " + std::to_string(m_lastTouchX) + ", " +
            // std::to_string(m_lastTouchY) + "."); libsystem::log("[TRACKPAD]
            // Origin Touch : " + std::to_string(originTouchX) + ", " +
            // std::to_string(originTouchY) + ".");

            if (m_trackpadTicks < UINT8_MAX)
                m_trackpadTicks++;

            if (isTrackpadActive())
            {
                if (m_trackpadTicks == 10)
                {
                    // Do once, only when trackpad was just enabled

                    // libsystem::log("[TRACKPAD] Reset.");

                    m_trackpadActiveBox->enable();

                    m_trackpadLastDeltaX = 0;

                    localGraphicalUpdate();
                }

                const int32_t deltaX = rawTouchX - originTouchX;
                std::string deltaXString = std::to_string(deltaX);

                constexpr int32_t stepsByChar = 8;
                const int32_t toMove = (deltaX - m_trackpadLastDeltaX) / stepsByChar;

                // libsystem::log("[TRACKPAD] Delta X : " + std::to_string(deltaX) +
                // "."); libsystem::log("[TRACKPAD] To Move : " +
                // std::to_string(toMove) + ".");

                if (toMove > 0)
                {
                    for (int i = 0; i < toMove; i++)
                    {
                        m_label->setCursorIndex(
                            static_cast<int16_t>(m_label->getCursorIndex() + 1)
                        );
                    }
                }
                else if (toMove < 0)
                {
                    for (int i = 0; i < -toMove; i++)
                    {
                        m_label->setCursorIndex(
                            static_cast<int16_t>(m_label->getCursorIndex() - 1)
                        );
                    }
                }

                if (abs(toMove) > 0)
                {
                    m_label->forceUpdate();
                    // m_trackpadActiveBox->forceUpdate();
                }

                m_trackpadLastDeltaX += toMove * stepsByChar;
            }
        }
        else
        {
            m_trackpadTicks = 0;

            if (wasTrackpadActive)
            {
                // Do once

                // m_trackpadFilter->disable();
                m_trackpadActiveBox->disable();

                localGraphicalUpdate();
            }
        }
    }

    bool Keyboard::isPointInTrackpad(const int16_t x, const int16_t y) const
    {
        if (x < 110 || x > 370)
            return false;
        if (y <= 260 || y > 300)
            return false;

        return true;
    }

    bool Keyboard::isTrackpadActive() const
    {
        return m_trackpadTicks >= 10;
    }

    void Keyboard::addChar(const char value)
    {
        m_buffer.insert(m_label->getCursorIndex(), 1, value);

        // Update cursor position
        // TODO: Remove m_buffer and use only label ?
        m_label->setText(m_buffer);
        m_label->setCursorIndex(m_label->getCursorIndex() + 1);
    }

    void Keyboard::removeChar()
    {
        if (m_buffer.empty())
            return;
        if (m_label->getCursorIndex() <= 0)
            return;

        m_buffer.erase(m_label->getCursorIndex() - 1, 1);

        m_label->setCursorIndex(static_cast<int16_t>(m_label->getCursorIndex() - 1));
    }
} // namespace gui::elements
