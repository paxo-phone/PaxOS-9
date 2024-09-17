//
// Created by Charles on 13/03/2024.
//

#include "Keyboard2.hpp"

#include <iostream>
#include <graphics.hpp>
#include <libsystem.hpp>
#include <Surface.hpp>

#include "Box.hpp"
#include "Filter.hpp"
#include "Image.hpp"

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

namespace gui::elements {
    Keyboard2::Keyboard2(const std::string &defaultText) {
        m_buffer = defaultText;
        m_defaultText = defaultText;

        // check with the graphics Orientation ... maybe add a parameter to choose ?
        m_width = graphics::getScreenWidth();
        m_height = graphics::getScreenHeight();

        m_x = 0;
        m_y = 0;

        m_backgroundColor = graphics::packRGB565(255, 255, 255);
        m_hasEvents = true;
        m_caps = CAPS_NONE;

        boxHeight = 40;
        hauteurKeyboard = boxHeight * 4;
        offsetBord = 5;
        keyZoomRatio = 1.5;
        hauteurRetourZoom = 10;

        
        int16_t largeur9 = (m_width - (9-1)) / static_cast<float>(9);
        int largeurConfirmation = 40;

//        m_keysCanvas = new Canvas(30, 140, 420, 160);
        m_keysCanvas = new Canvas(offsetBord, m_height - hauteurKeyboard -offsetBord, m_width-2*offsetBord, hauteurKeyboard);
        addChild(m_keysCanvas);

        m_layoutLowercase = new char *[4];
        m_layoutLowercase[0] = new char[10]{'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'};
        m_layoutLowercase[1] = new char[10]{'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm'};
        m_layoutLowercase[2] = new char[9]{KEY_CAPS, 'w', 'x', 'c', 'v', 'b', 'n', '\'', KEY_BACKSPACE};
        m_layoutLowercase[3] = new char[3]{KEY_LAYOUT_NUMBERS, KEY_SPACE, KEY_EXIT};

        m_layoutUppercase = new char *[4];
        m_layoutUppercase[0] = new char[10]{'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'};
        m_layoutUppercase[1] = new char[10]{'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M'};
        m_layoutUppercase[2] = new char[9]{KEY_CAPS, 'W', 'X', 'C', 'V', 'B', 'N', '\'', KEY_BACKSPACE};
        m_layoutUppercase[3] = new char[3]{KEY_LAYOUT_NUMBERS, KEY_SPACE, KEY_EXIT};

        m_layoutNumbers = new char *[4];
        m_layoutNumbers[0] = new char[10]{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
        m_layoutNumbers[1] = new char[10]{'+', '-', '*', '/', '(', ')', '[', ']', '<', '>'};
        m_layoutNumbers[2] = new char[9]{KEY_CAPS, '_', ',', '.', ':', ';', '!', '?', KEY_BACKSPACE};
        m_layoutNumbers[3] = new char[3]{KEY_LAYOUT_STANDARD, KEY_SPACE, KEY_EXIT};

        m_currentLayout = LAYOUT_LOWERCASE;

        // Create label for text
          m_label = new Label(offsetBord+1, offsetBord, m_width-2*offsetBord - largeurConfirmation, m_height - hauteurKeyboard-2*offsetBord);
        // m_label->setFont(graphics::ARIAL);
        m_label->setFontSize(24);
        m_label->setCursorEnabled(true);
        m_label->setText(m_buffer);
        m_label->setCursorIndex(static_cast<int16_t>(m_buffer.length()));
        addChild(m_label);

        // Create images box (for better performances ?)

        m_capsBox = new Box(offsetBord+1, m_height-hauteurKeyboard-offsetBord +boxHeight*2+1, largeur9-2, boxHeight-2);
        m_backspaceBox = new Box(m_width-offsetBord-largeur9+2, m_height-hauteurKeyboard-offsetBord+boxHeight*2+1, largeur9-2, boxHeight-2);

        m_confirmBox = new Box(m_width-largeurConfirmation -offsetBord, offsetBord, largeurConfirmation, boxHeight);

        m_layoutBox = new Box(offsetBord+1, m_height-hauteurKeyboard-offsetBord+boxHeight*3+1, 60-2, boxHeight-2);
        m_exitBox = new Box(m_width-offsetBord-60+1, m_height-hauteurKeyboard-offsetBord+boxHeight*3+1, 60-2, boxHeight-2);

        m_capsBox->setBackgroundColor(COLOR_LIGHT_GREY);
        m_capsBox->setRadius(5);
        m_layoutBox->setBackgroundColor(COLOR_LIGHT_GREY);
        m_layoutBox->setRadius(5);
        m_backspaceBox->setBackgroundColor(COLOR_LIGHT_GREY);
        m_backspaceBox->setRadius(5);
        m_exitBox->setBackgroundColor(COLOR_LIGHT_GREY);
        m_exitBox->setRadius(5);



        // Gestion des key Zoom 
        const float keyWidth10 = (m_width - (10-1)) / 10 - 1;
        const float keyWidth9 = (m_width - (9-1)) / 9 - 1;

        // ZoomKey Left 10
        m_boxdKeyPressed_Left10 = new Box (0, 0, keyWidth10*keyZoomRatio, boxHeight*keyZoomRatio);
        m_boxdKeyPressed_Left10-> disable();
        m_CanevasKeyPressed_Left10 = new Canvas(0,0,keyWidth10*keyZoomRatio, boxHeight*keyZoomRatio);
        m_CanevasKeyPressed_Left10->fillRect(0,0, keyWidth10*keyZoomRatio, boxHeight*keyZoomRatio, COLOR_WHITE);

        std::vector<std::pair<int16_t, int16_t>> verticesLeft10;
        verticesLeft10.push_back( {0                         , 5} );
        verticesLeft10.push_back( {0                         , boxHeight*keyZoomRatio} );
        verticesLeft10.push_back( {keyWidth10-2              , boxHeight*keyZoomRatio} );
        verticesLeft10.push_back( {keyWidth10*keyZoomRatio-1   , boxHeight*keyZoomRatio-hauteurRetourZoom });
        verticesLeft10.push_back( {keyWidth10*keyZoomRatio-1   , 5} );
        verticesLeft10.push_back( {keyWidth10*keyZoomRatio-5-1 , 0} );
        verticesLeft10.push_back( {5                         , 0} );

        m_CanevasKeyPressed_Left10->fillPolygon(verticesLeft10, COLOR_LIGHT_GREY);
        m_CanevasKeyPressed_Left10->drawPolygon(verticesLeft10, COLOR_WHITE);
        m_boxdKeyPressed_Left10->addChild(m_CanevasKeyPressed_Left10);
        addChild(m_boxdKeyPressed_Left10);

        // ZoomKey Center 10
        m_boxdKeyPressed_Center10 = new Box (0, 0, keyWidth10*keyZoomRatio, boxHeight*keyZoomRatio);
        m_boxdKeyPressed_Center10-> disable();
        m_CanevasKeyPressed_Center10 = new Canvas(0,0,keyWidth10*keyZoomRatio, boxHeight*keyZoomRatio);
        m_CanevasKeyPressed_Center10->fillRect(0,0, keyWidth10*keyZoomRatio, boxHeight*keyZoomRatio, COLOR_WHITE);

        std::vector<std::pair<int16_t, int16_t>> verticesCenter10;
        verticesCenter10.push_back( {0                         , 5} );
        verticesCenter10.push_back( {0                         , boxHeight*keyZoomRatio-hauteurRetourZoom} );
        verticesCenter10.push_back( {(keyWidth10*keyZoomRatio - keyWidth10 ) /2 , boxHeight*keyZoomRatio} );
        verticesCenter10.push_back( {(keyWidth10*keyZoomRatio - keyWidth10 ) /2 +keyWidth10, boxHeight*keyZoomRatio} );
        verticesCenter10.push_back( {keyWidth10*keyZoomRatio-1   , boxHeight*keyZoomRatio-hauteurRetourZoom });
        verticesCenter10.push_back( {keyWidth10*keyZoomRatio-1   , 5} );
        verticesCenter10.push_back( {keyWidth10*keyZoomRatio-5-1 , 0} );
        verticesCenter10.push_back( {5                         , 0} );

        m_CanevasKeyPressed_Center10->fillPolygon(verticesCenter10, COLOR_LIGHT_GREY);
        m_CanevasKeyPressed_Center10->drawPolygon(verticesCenter10, COLOR_WHITE);
        m_boxdKeyPressed_Center10->addChild(m_CanevasKeyPressed_Center10);
        addChild(m_boxdKeyPressed_Center10);

        // ZoomKey Right 10
        m_boxdKeyPressed_Right10 = new Box (0, 0, keyWidth10*keyZoomRatio, boxHeight*keyZoomRatio);
        m_boxdKeyPressed_Right10-> disable();
        m_CanevasKeyPressed_Right10 = new Canvas(0,0,keyWidth10*keyZoomRatio, boxHeight*keyZoomRatio);
        m_CanevasKeyPressed_Right10->fillRect(0,0, keyWidth10*keyZoomRatio, boxHeight*keyZoomRatio, COLOR_WHITE);

        std::vector<std::pair<int16_t, int16_t>> verticesRight10;
        verticesRight10.push_back( {0                                       , 5} );
        verticesRight10.push_back( {0                                       , boxHeight*keyZoomRatio-hauteurRetourZoom} );
        verticesRight10.push_back( {keyWidth10*keyZoomRatio - keyWidth10+1  , boxHeight*keyZoomRatio} );
        verticesRight10.push_back( {keyWidth10*keyZoomRatio-1                 , boxHeight*keyZoomRatio} );
        verticesRight10.push_back( {keyWidth10*keyZoomRatio-1                 , 5 });
        verticesRight10.push_back( {keyWidth10*keyZoomRatio-5-1               , 0} );
        verticesRight10.push_back( {5                                       , 0} );

        m_CanevasKeyPressed_Right10->fillPolygon(verticesRight10, COLOR_LIGHT_GREY);
        m_CanevasKeyPressed_Right10->drawPolygon(verticesRight10, COLOR_WHITE);
        m_boxdKeyPressed_Right10->addChild(m_CanevasKeyPressed_Right10);
        addChild(m_boxdKeyPressed_Right10);

        // ZoomKey Center 9
        m_boxdKeyPressed_Center9 = new Box (0, 0, keyWidth9*keyZoomRatio, boxHeight*keyZoomRatio);
        m_boxdKeyPressed_Center9-> disable();
        m_CanevasKeyPressed_Center9 = new Canvas(0,0,keyWidth9*keyZoomRatio, boxHeight*keyZoomRatio);
        m_CanevasKeyPressed_Center9->fillRect(0,0, keyWidth9*keyZoomRatio, boxHeight*keyZoomRatio, COLOR_WHITE);

        std::vector<std::pair<int16_t, int16_t>> verticesCenter9;
        verticesCenter9.push_back( {0                         , 5} );
        verticesCenter9.push_back( {0                         , boxHeight*keyZoomRatio-hauteurRetourZoom} );
        verticesCenter9.push_back( {(keyWidth9*keyZoomRatio - keyWidth9 ) /2 , boxHeight*keyZoomRatio} );
        verticesCenter9.push_back( {(keyWidth9*keyZoomRatio - keyWidth9 ) /2 +keyWidth9, boxHeight*keyZoomRatio} );
        verticesCenter9.push_back( {keyWidth9*keyZoomRatio-1   , boxHeight*keyZoomRatio-hauteurRetourZoom });
        verticesCenter9.push_back( {keyWidth9*keyZoomRatio-1   , 5} );
        verticesCenter9.push_back( {keyWidth9*keyZoomRatio-1-5 , 0} );
        verticesCenter9.push_back( {5                         , 0} );

        m_CanevasKeyPressed_Center9->fillPolygon(verticesCenter9, COLOR_LIGHT_GREY);
        m_CanevasKeyPressed_Center9->drawPolygon(verticesCenter9, COLOR_WHITE);
        m_boxdKeyPressed_Center9->addChild(m_CanevasKeyPressed_Center9);
        addChild(m_boxdKeyPressed_Center9);
/*
        m_pressedKey_Right10;
        m_pressedKey_Left9;
        m_pressedKey_Left9;
        m_pressedKey_Center10;        
        m_pressedKey_Center9;
*/

        // Create images
        m_capsIcon0     = new Image(storage::Path("system/Keyboard2/caps_icon_0.png"), 3, 3, largeur9-7, boxHeight-7, COLOR_LIGHT_GREY);
        m_capsIcon1     = new Image(storage::Path("system/Keyboard2/caps_icon_1.png"), 3, 3, largeur9-7, boxHeight-7, COLOR_LIGHT_GREY);
        m_capsIcon2     = new Image(storage::Path("system/Keyboard2/caps_icon_2.png"), 3, 3, largeur9-7, boxHeight-7, COLOR_LIGHT_GREY);
        m_backspaceIcon = new Image(storage::Path("system/Keyboard2/backspace_icon.png"), 3, 3, largeur9-7, boxHeight-7, COLOR_LIGHT_GREY);

        m_layoutIcon0   = new Image(storage::Path("system/Keyboard2/layout_icon_0.png"), 10, 3, 40, boxHeight-7, COLOR_LIGHT_GREY);
        m_layoutIcon1   = new Image(storage::Path("system/Keyboard2/layout_icon_1.png"), 10, 3, 40, boxHeight-7, COLOR_LIGHT_GREY);

        m_exitIcon      = new Image(storage::Path("system/Keyboard2/exit_icon.png"), 3, 3, 60-4, boxHeight-7, COLOR_LIGHT_GREY);
        m_confirmIcon   = new Image(storage::Path("system/Keyboard2/confirm_icon.png"), 0, 0, largeurConfirmation, boxHeight);

        // Load images into RAM
        m_capsIcon0->load();
        m_capsIcon1->load();
        m_capsIcon2->load();
        m_backspaceIcon->load();
        m_layoutIcon0->load();
        m_layoutIcon1->load();
        m_exitIcon->load();
        m_confirmIcon->load();

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

//        m_trackpadActiveBox = new Box(192, 112, 96, 96);
        m_trackpadActiveBox = new Box((m_width-96) /2, m_height-offsetBord-hauteurKeyboard+12, 96, 96);
        m_trackpadActiveBox->setBackgroundColor(TFT_BLACK);
        m_trackpadActiveBox->setRadius(8);
        addChild(m_trackpadActiveBox);

        m_trackpadActiveIcon = new Image(storage::Path("system/Keyboard2/trackpad_active_icon.png"), 16, 16, 64, 64);
        m_trackpadActiveIcon->load(TFT_BLACK);
        m_trackpadActiveBox->addChild(m_trackpadActiveIcon);

        m_trackpadActiveBox->disable();

        m_trackpadTicks = 0;
        m_trackpadLastDeltaX = 0;
        m_zoomKeyTicks = 0;
    }



    Keyboard2::~Keyboard2() = default;



    void Keyboard2::render() {
        m_surface->fillRect(0, 0, m_width, m_height, m_backgroundColor);

        // Input box
        drawInputBox();

        if (!isTrackpadActive()) {
            // Draw keys
            drawKeys();
        }
    }

    void Keyboard2::widgetUpdate() {
        if (isTouched()) {
            // Get touch position
            int16_t touchX, touchY;
            getLastTouchPosRel(&touchX, &touchY);

            const char pressedKey = getKey(touchX, touchY);
            if (pressedKey == KEY_NULL) {
                return;
            }

            //drawZoomKeyLeft10 (touchX, touchY, pressedKey, 10);
            processKey(pressedKey);
        }

        if (m_exitBox->isTouched()) {
            m_buffer = m_defaultText; // Reset text
            m_exit = true;
        }

        if (m_confirmBox->isTouched()) {
            m_exit = true;
        }

        if (m_backspaceBox->isTouched()) {
            removeChar();

            // Redraw input box
            drawInputBox();
        }

        if (m_capsBox->isTouched()) {
            switch (m_caps) {
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

        if (m_layoutBox->isTouched()) {
            if (m_currentLayout == LAYOUT_LOWERCASE || m_currentLayout == LAYOUT_UPPERCASE) {
                m_currentLayout = LAYOUT_NUMBERS;
            } else if (m_currentLayout == LAYOUT_NUMBERS) {
                if (m_caps == CAPS_NONE) {
                    m_currentLayout = LAYOUT_LOWERCASE;
                } else {
                    m_currentLayout = LAYOUT_UPPERCASE;
                }
            }

            drawKeys();
            updateLayoutIcon();
        }

        trackpadUpdate();
        zoomKeyUpdate();
    }

    std::string Keyboard2::getText() {
        const std::string output = m_buffer;

        m_buffer = "";

        return output;
    }

    void Keyboard2::drawKeys() const {
        // Reset default settings
        m_keysCanvas->fillRect(0, 0, m_keysCanvas->getWidth(), m_keysCanvas->getHeight(),
//                               graphics::packRGB565(125, 126, 127));
                                graphics::packRGB565(255, 255, 255));

        // Draw every keys
        drawKeyRow(0*boxHeight, getLayoutCharMap()[0]);
        drawKeyRow(1*boxHeight, getLayoutCharMap()[1]);
        drawKeyRow(2*boxHeight, getLayoutCharMap()[2]);
        drawLastRow(3*boxHeight);
    }

    void Keyboard2::drawKeyRow(const int16_t y, const char *keys) const {

        uint8_t count = strlen(keys);
        // calcul de la largeur d'une touche (on laisse 1 entre chacune touche)
        const float keyWidth = (m_width - (count-1)) / static_cast<float>(count);

        for (uint16_t i = 0; i < count; i++) {
            drawKey(
                static_cast<int16_t>(static_cast<float>(i) * keyWidth), // position X
                y,                                                      // position Y
                static_cast<int16_t>(keyWidth),                         // largeur touche
                keys[i]                                                 // Texte de la touche
            );
        }
    }

    void Keyboard2::drawKey(const int16_t x, const int16_t y, const uint16_t w, const char key) const {
        auto keyString = std::string(1, key);
//        const float keyWidth = (m_width - (count-1)) / static_cast<float>(count);

        m_keysCanvas->fillRoundRect(x+1, y+1, w-2, boxHeight-2, 5, COLOR_LIGHT_GREY);
        m_keysCanvas->drawTextCenteredInRect(x+1, y+1, w-2, boxHeight-2, keyString, COLOR_BLACK, true, true, 28);
    }

    void Keyboard2::drawLastRow(const int16_t y) const {
        // Draw spacebar
        m_keysCanvas->fillRoundRect(60+1, y+1, m_width-2*60-2*offsetBord-2, boxHeight-2, 5, COLOR_LIGHT_GREY);
        m_keysCanvas->fillRect(offsetBord+60+5, y+3*boxHeight/4, m_width-(2*60+2*offsetBord+20), 2, graphics::packRGB565(0, 0, 0));
    }

        uint8_t Keyboard2::getKeyCol(const int16_t x, const uint8_t keyCount) const {
        float boxX = offsetBord;
//        const float keyWidth = 420.0f / static_cast<float>(keyCount);
        const float keyWidth = (m_width-2*offsetBord) / static_cast<float>(keyCount);

        for (uint8_t i = 0; i < keyCount; i++) {
            if (static_cast<float>(x) >=  boxX && static_cast<float>(x) <= boxX + keyWidth) {
                return i;
            }
            boxX += keyWidth;
        }
        return -1;
    }

    char Keyboard2::getKey(const int16_t x, const int16_t y) const {
        // Check if the position is in the Keyboard2 box


        Keyboard2::Coord result;
        result = getRowColumn(x, y);

        if (result.row == -1 && result.column == -1) {
            return KEY_NULL;
        }

/*

        std::cout << "x = " << x << " y = " << y << std::endl;
        if (!(x >= offsetBord && x <= m_width-offsetBord && y >= m_height - hauteurKeyboard - offsetBord && y <= m_height - offsetBord)) {
            std::cout << "NULL KEY"<< std::endl;
            return KEY_NULL;
        }                                

        int row;
        int column;

        row =  3 - (int) (m_height -  y - offsetBord)/boxHeight;
        if (row <3) {
            column = getKeyCol(x, strlen(getLayoutCharMap()[row]));

        } else {
            // Get column of last row
            if (x <= 60 + offsetBord) {
                column = 0;
            } else if (x > m_width - offsetBord-60) {
                column = 2;
            } else {
                column = 1;
            }
        }
*/
        return getLayoutCharMap()[result.row][result.column];
    }


    Keyboard2::Coord Keyboard2::getRowColumn(const int16_t x, const int16_t y) const {
        // Check if the position is in the Keyboard2 box
//        if (!(x >= 30 && x <= 450 && y >= 140 && y <= 300)) {
        Coord result;
        result.row = -1;
        result.column = -1;
//        std::cout << "[getRowColumn]  x=" << x << " y=" << y << std::endl;


        if (!(x >= offsetBord && x <= m_width-offsetBord && y >= m_height - hauteurKeyboard - offsetBord && y <= m_height - offsetBord)) {
//            std::cout << "[getRowColumn] Hors Champs x=" << x << " y=" << y <<std::endl;
            return result;
        }                                

        uint16_t row;
        uint16_t column;

        row =  3 - (uint16_t) (m_height -  y - offsetBord)/boxHeight;
//        std::cout << "[getRowColumn] row=" << row <<std:: endl;

        if (row <3) {
            column = getKeyCol(x, strlen(getLayoutCharMap()[row]));

        } else {
            // Get column of last row
            if (x <= 60 + offsetBord) {
                column = 0;
            } else if (x > m_width - offsetBord-60) {
                column = 2;
            } else {
                column = 1;
            }
        }
//        std::cout << "[getRowColumn] column=" << column <<std:: endl;

        result.row = row;
        result.column = column;

        return result;
    }



    /**
     * Execute the needed action for the key
     * @param key The key to process
     */
    void Keyboard2::processKey(const char key) {
        switch (key) {
            case KEY_NULL:
            case KEY_EXIT:
            case KEY_BACKSPACE:
            case KEY_CAPS:
            case KEY_LAYOUT_STANDARD:
            case KEY_LAYOUT_NUMBERS:
                // KEY_EXIT & KEY_BACKSPACE & KEY_CAPS & KEY_LAYOUT_STANDARD & KEY_LAYOUT_NUMBERS are handled directly in update function
                return;
            case KEY_SPACE:
                addChar(' ');
                break;
            default:
                addChar(key);

            // Disable caps if not locked
                if (m_caps == CAPS_ONCE) {
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

    void Keyboard2::drawInputBox() const {
        if (m_buffer.empty()) {
            if (m_placeholder.empty()) {
                m_label->setText("");
                return;
            }

            // Draw placeholder
            m_label->setTextColor(graphics::packRGB565(200, 200, 200));
            m_label->setText(m_placeholder);

            m_label->setCursorEnabled(false);
        } else {
            // Draw text
            m_label->setTextColor(graphics::packRGB565(0, 0, 0));
            m_label->setText(m_buffer);

            m_label->setCursorEnabled(true);
        }
    }


    void Keyboard2::updateCapsIcon() const {
        switch (m_caps) {
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
            default: ;
        }
    }

    void Keyboard2::updateLayoutIcon() const {
        switch (m_currentLayout) {
            case LAYOUT_LOWERCASE:
            case LAYOUT_UPPERCASE:
                m_layoutIcon0->enable();
                m_layoutIcon1->disable();
                break;
            case LAYOUT_NUMBERS:
                m_layoutIcon0->disable();
                m_layoutIcon1->enable();
                break;
            default: ;
        }
    }

    bool Keyboard2::hasExitKeyBeenPressed() const {
        return m_exit;
    }

    void Keyboard2::setPlaceholder(const std::string &placeholder) {
        m_placeholder = placeholder;
    }

    char **Keyboard2::getLayoutCharMap() const {
        switch (m_currentLayout) {
            case LAYOUT_LOWERCASE:
            default:
                return m_layoutLowercase;
            case LAYOUT_UPPERCASE:
                return m_layoutUppercase;
            case LAYOUT_NUMBERS:
                return m_layoutNumbers;
        }
    }

    void Keyboard2::trackpadUpdate() {
        int16_t rawTouchX, rawTouchY;
        graphics::getTouchPos(&rawTouchX, &rawTouchY);

        const bool wasTrackpadActive = isTrackpadActive();

        // Check if finger is on screen
        if ((rawTouchX != -1 && rawTouchY != -1) && isPointInTrackpad(originTouchX, originTouchY)) {

            if (m_trackpadTicks < UINT8_MAX) {
                m_trackpadTicks++;
            }

            if (isTrackpadActive()) {
                if (m_trackpadTicks == 10) {
                    m_trackpadActiveBox->enable();
                    m_trackpadLastDeltaX = 0;
                    localGraphicalUpdate();
                }

                const int32_t deltaX = rawTouchX - originTouchX;
                std::string deltaXString = std::to_string(deltaX);

                constexpr int32_t stepsByChar = 8;
                const int32_t toMove = (deltaX - m_trackpadLastDeltaX) / stepsByChar;

                if (toMove > 0) {
                    for (int i = 0; i < toMove; i++) {
                        m_label->setCursorIndex(static_cast<int16_t>(m_label->getCursorIndex() + 1));
                    }
                } else if (toMove < 0) {
                    for (int i = 0; i < -toMove; i++) {
                        m_label->setCursorIndex(static_cast<int16_t>(m_label->getCursorIndex() - 1));
                    }
                }

                if (abs(toMove) > 0) {
                    m_label->forceUpdate();
                    // m_trackpadActiveBox->forceUpdate();
                }

                m_trackpadLastDeltaX += toMove * stepsByChar;
            }
        } else {
            m_trackpadTicks = 0;

            if (wasTrackpadActive) {
                // Do once
                // m_trackpadFilter->disable();
                m_trackpadActiveBox->disable();
                localGraphicalUpdate();
            }
        }
    }

    bool Keyboard2::isPointInTrackpad(const int16_t x, const int16_t y) const {

        if (x < 60+1 || x > 60+1 + m_width-2*60-2*offsetBord-2) return false;
        if (y< m_height -offsetBord- boxHeight || y > m_height - offsetBord) return false;

        return true;
    }

    bool Keyboard2::isTrackpadActive() const {
        return m_trackpadTicks >= 10;
    }

    void Keyboard2::addChar(const char value) {
        m_buffer.insert(m_label->getCursorIndex(), 1, value);

        // Update cursor position
        // TODO: Remove m_buffer and use only label ?
        m_label->setText(m_buffer);
        m_label->setCursorIndex(m_label->getCursorIndex() + 1);
    }

    void Keyboard2::removeChar() {
        if (m_buffer.empty()) {
            return;
        }
        if (m_label->getCursorIndex() <= 0) {
            return;
        }

        m_buffer.erase(m_label->getCursorIndex() - 1, 1);
        m_label->setCursorIndex(static_cast<int16_t>(m_label->getCursorIndex() - 1));
    }



    void Keyboard2::drawZoomKeyLeft10 (uint16_t x, uint16_t y,uint16_t keyWidth , char key)
    {

        // Create Box for the current pressed key
        m_boxdKeyPressed_Left10->enable();
        m_boxdKeyPressed_Left10->setX(x);
        m_boxdKeyPressed_Left10->setY(y);
        
        auto keyString = std::string(1, key);
        m_CanevasKeyPressed_Left10->fillRect(3,3,keyWidth*keyZoomRatio-7, boxHeight*keyZoomRatio-hauteurRetourZoom, COLOR_LIGHT_GREY);
        m_CanevasKeyPressed_Left10->drawTextCenteredInRect(3,3,keyWidth*keyZoomRatio-7, boxHeight*keyZoomRatio-hauteurRetourZoom, keyString, COLOR_BLACK, true, true, 38);


    }

 void Keyboard2::drawZoomKeyCenter10 (uint16_t x, uint16_t y,uint16_t keyWidth , char key)
    {

        // Create Box for the current pressed key
        m_boxdKeyPressed_Center10->enable();
        m_boxdKeyPressed_Center10->setX(x);
        m_boxdKeyPressed_Center10->setY(y);
        
        auto keyString = std::string(1, key);
        m_CanevasKeyPressed_Center10->fillRect(3,3,keyWidth*keyZoomRatio-7, boxHeight*keyZoomRatio-hauteurRetourZoom, COLOR_LIGHT_GREY);
        m_CanevasKeyPressed_Center10->drawTextCenteredInRect(3,3,keyWidth*keyZoomRatio-7, boxHeight*keyZoomRatio-hauteurRetourZoom, keyString, COLOR_BLACK, true, true, 38);

    }

    void Keyboard2::drawZoomKeyRight10 (uint16_t x, uint16_t y,uint16_t keyWidth , char key)
    {

        // Create Box for the current pressed key
        m_boxdKeyPressed_Right10->enable();
        m_boxdKeyPressed_Right10->setX(x);
        m_boxdKeyPressed_Right10->setY(y);
        
        auto keyString = std::string(1, key);
        m_CanevasKeyPressed_Right10->fillRect(3,3,keyWidth*keyZoomRatio-7, boxHeight*keyZoomRatio-hauteurRetourZoom, COLOR_LIGHT_GREY);
        m_CanevasKeyPressed_Right10->drawTextCenteredInRect(3,3,keyWidth*keyZoomRatio-7, boxHeight*keyZoomRatio-hauteurRetourZoom, keyString, COLOR_BLACK, true, true, 38);

    }
    
 void Keyboard2::drawZoomKeyCenter9 (uint16_t x, uint16_t y,uint16_t keyWidth , char key)
    {

        // Create Box for the current pressed key
        m_boxdKeyPressed_Center9->enable();
        m_boxdKeyPressed_Center9->setX(x);
        m_boxdKeyPressed_Center9->setY(y);
        
        auto keyString = std::string(1, key);
        m_CanevasKeyPressed_Center9->fillRect(3,3,keyWidth*keyZoomRatio-7, boxHeight*keyZoomRatio-25, COLOR_LIGHT_GREY);
        m_CanevasKeyPressed_Center9->drawTextCenteredInRect(3,3,keyWidth*keyZoomRatio-7, boxHeight*keyZoomRatio-25, keyString, COLOR_BLACK, true, true, 38);

    }
    void Keyboard2::zoomKeyUpdate() {

        int16_t rawTouchX, rawTouchY;
        graphics::getTouchPos(&rawTouchX, &rawTouchY);

        const bool wasZoomActive = isZoomdActive();

        char pressedKey = KEY_NULL;

        // Check if finger is on screen and on a key
        if (rawTouchX != -1 && rawTouchY != -1) {
            //pressedKey = getKey(rawTouchX, rawTouchY);

            Keyboard2::Coord pressedKeyCoord = getRowColumn(rawTouchX, rawTouchY);

             if (pressedKeyCoord.row == -1 && pressedKeyCoord.column == -1) {
                //repressedKeyCoordturn KEY_NULL;
                return;
            }   

            oldCoord.row = pressedKeyCoord.row;
            oldCoord.column = pressedKeyCoord.column;

            pressedKey= getLayoutCharMap()[pressedKeyCoord.row][pressedKeyCoord.column];

            if (m_zoomKeyTicks < UINT8_MAX) {
                m_zoomKeyTicks++;
            }            

            if (isZoomdActive()) {
                if (m_zoomKeyTicks == 10) {
                    enableKeyZoom(pressedKeyCoord.row, pressedKeyCoord.column, pressedKey);
                    localGraphicalUpdate();
                }
            }
        } else {
            m_zoomKeyTicks = 0;

            if (wasZoomActive) {
                // Do once
                // m_trackpadFilter->disable();
                disableKeyZoom(oldCoord.row , oldCoord.column);
                localGraphicalUpdate();
            }
        }
    }


    bool Keyboard2::isZoomdActive() const {
        return m_zoomKeyTicks >= 10;
    }


    void Keyboard2::enableKeyZoom(uint16_t row, uint16_t column, char pressedKey) 
    {

        if (row >=0 and row <2) {
            int count = 10;
            const float keyWidth = (m_width - (count-1)) / static_cast<float>(count)-1;
            int x = (keyWidth+1) * column + offsetBord +1;
            int y = m_height - hauteurKeyboard -offsetBord - boxHeight*keyZoomRatio + row * boxHeight +1+5;

            if (column ==0 ){
                drawZoomKeyLeft10(x-1, y, keyWidth, pressedKey);
            } else if (column <9) {
                drawZoomKeyCenter10(x-(keyWidth*keyZoomRatio - keyWidth)/2, y, keyWidth, pressedKey);
            } else if (column ==9) {
                drawZoomKeyRight10(x-(keyWidth*keyZoomRatio - keyWidth)+1, y, keyWidth, pressedKey);
            }
        }
        else if (row ==2 && column >0 && column<8)
        {
            int count = 9;
            const float keyWidth = (m_width - (count-1)) / static_cast<float>(count)-1;
            int x = (keyWidth+1) * column + offsetBord +1;
            int y = m_height - hauteurKeyboard -offsetBord - boxHeight*keyZoomRatio + row * boxHeight +1+5;
            drawZoomKeyCenter9(x-(keyWidth*keyZoomRatio - keyWidth)/2, y, keyWidth, pressedKey);
        }
    }
    
    void Keyboard2::disableKeyZoom(uint16_t row, uint16_t column)
    {
        if (row >=0 and row <2) {
            if (column ==0 ){
                m_boxdKeyPressed_Left10->disable();
            }
            else if (column<9)
            {
                m_boxdKeyPressed_Center10->disable();
            }
            else if (column==9)
            {
                m_boxdKeyPressed_Right10->disable();
            }
        }
        else if (row == 2){
            if (column == 0 || column == 8) return;
            {
                m_boxdKeyPressed_Center9->disable();
            }
        }
    }





} // gui::elements


