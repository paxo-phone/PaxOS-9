#ifndef ELEMENTBASE_HPP
#define ELEMENTBASE_HPP

#define USE_DOUBLE_BUFFERING

#include "Surface.hpp"

#include <cstdint> // for uint16_t
#include <vector>

typedef uint16_t color_t; // @Charles a remplacer quand tu auras mis la lib graphique
#ifdef USE_DOUBLE_BUFFERING
#define SCROLL_STEP 1
#else
#define SCROLL_STEP 20
#endif

namespace gui
{
    class ElementBase
    {
      public:
        ElementBase();
        virtual ~ElementBase();

        /**
         * @brief Reset global states of the GUI.
         */
        static void resetStates();

        /**
         * @brief The render function is called to draw the widget on the screen.
         */
        virtual void render() = 0;

        /**
         * @brief Render the widget and all its children.
         * @param onScreen If true, the widget will be rendered on the screen.
         */
        void renderAll(bool onScreen = true);

        /**
         * @brief Function called before the render function.
         */
        virtual void preRender() {}

        /**
         * @brief Function called after the render function.
         */
        virtual void postRender() {}

        /**
         * @brief Update the widget and all its children.
         * @return True if the widget or one of its children has an event to process.
         */
        bool updateAll();

        /**
         * @brief Update the widget.
         * @return True if the widget has an event to process.
         */
        bool update();

        /**
         * @brief Function called to update the widget.
         */
        virtual void widgetUpdate() {};

        void setX(uint16_t x);
        void setY(uint16_t y);
        void setWidth(uint16_t width);
        void setHeight(uint16_t height);

        [[nodiscard]] int16_t getX() const;
        [[nodiscard]] int16_t getY() const;
        [[nodiscard]] uint16_t getWidth() const;
        [[nodiscard]] uint16_t getHeight() const;

        [[nodiscard]] int16_t getAbsoluteX() const;
        [[nodiscard]] int16_t getAbsoluteY() const;

        virtual void setBackgroundColor(color_t color);
        virtual void setBorderColor(color_t color);

        void setRadius(uint16_t r);
        [[nodiscard]] uint16_t getRadius() const;

        void setBorderSize(uint16_t size);
        [[nodiscard]] uint16_t getBorderSize() const;

        [[nodiscard]] color_t getBackgroundColor() const;
        [[nodiscard]] color_t getBorderColor() const;

        /**
         * @brief Check if the widget is pressed.
         * Pressed means that the finger was on the widget and has been released.
         * @return
         */
        bool isTouched();

        /**
         * @brief Check if the widget is focused.
         * Focused means that the finger is currently on the widget.
         * @param forced If true, the function will do a manual check of the touch position
         * (skipping the touch event system).
         * @return True if the widget is focused, false otherwise.
         */
        bool isFocused(bool forced = false);

        virtual void onClick() {}

        virtual void onLongClick() {}

        /**
         * @brief When finger leave the widget and is considered as released
         */
        virtual void onReleased() {}

        /**
         * @brief When the widget is no longer considered as touched even if the
         * finger is still on the screen
         */
        virtual void onNotClicked() {}

        void enable();
        void disable();

        /**
         * @brief Free the memory used by the widget and its children.
         */
        void free();

        void setEnabled(bool enabled);
        [[nodiscard]] bool isEnabled() const;

        /**
         * @brief Get the highest parent widget in the hierachy
         * @return the highest parent in the hierarchy
         */
        ElementBase* getMaster();
        [[nodiscard]] ElementBase* getParent() const;
        void addChild(ElementBase* child);

        ElementBase* getElementAt(int index);

        ElementBase* parent_;
        std::vector<ElementBase*> children_;

        static int16_t touchX, touchY;
        static int16_t lastEventTouchX, lastEventTouchY;

        /**
         * Get the surface of the ElementBase.
         * @return A shared pointer to the surface.
         */
        std::shared_ptr<graphics::Surface> getSurface();

        void forceUpdate();

        uint16_t x_;
        uint16_t y_;

      protected:
        void freeRamFor(uint32_t size, ElementBase* window);

        // Position and size.
        uint16_t width_;
        uint16_t height_;

        // Colors.
        color_t backgroundColor_;
        color_t borderColor_;

        // Border size and radius.
        uint16_t borderSize_;
        uint16_t borderRadius_;

        // Hitbox widget increase factor (in pixels).
        uint16_t hitboxFactor_ = 10;

        // Scrolling related variables.
        static bool scrolling;
        bool verticalScrollEnabled_;
        bool horizontalScrollEnabled_;
        int16_t verticalScroll_ = 0;
        int16_t horizontalScroll_ = 0;

        // Update.
        bool isEnabled_;
        bool hasEvents_; // If it has events to process (touch, click, ...).

        // Render states.
        bool isRendered_; // Is buffer up-to-date?
        bool isDrawn_;    // Is up-to-date on screen?

        static ElementBase* masterOfRender;
        static ElementBase* mainWindow;

        enum PressedState
        {
            NOT_PRESSED,
            PRESSED,
            SLIDED,
            RELEASED,
            SCROLL_X,
            SCROLL_Y,
            LOCKED
        };

        PressedState pressedState_;

        static PressedState globalPressedState;
        static ElementBase* widgetPressed; // The widget that is currently pressed (or nullptr if none).
        static int16_t originTouchX;
        static int16_t originTouchY;

        ElementBase* getHighestXScrollableParent();
        ElementBase* getHighestYScrollableParent();

        /**
         * @brief Check if the widget is visible inside its parent.
         * @return True if the widget is inside its parent, false otherwise.
         */
        bool isInside();

        static int16_t lastTouchX;
        static int16_t lastTouchY;

        static bool wasPressed; // If the screen was pressed in the last update.

        void getLastTouchPosAbs(int16_t* x, int16_t* y) const;
        void getLastTouchPosRel(int16_t* x, int16_t* y) const;

        virtual void onScrollUp() {};
        virtual void onScrollDown() {};
        virtual void onScrollLeft() {};
        virtual void onScrollRight() {};
        virtual void onScroll(int16_t x, int16_t y) {};

        std::shared_ptr<graphics::Surface> surface_;         // Surface to render the widget
        std::shared_ptr<graphics::Surface> dmaSurface_; // double buffer for DMA transfer

        bool useDoubleBuffering_ = false;

        /**
         * @brief Get the surface of the ElementBase and initialize it if it is
         * nullptr.
         * @return A shared pointer to the surface.
         */
        std::shared_ptr<graphics::Surface> getAndSetSurface();

        void localGraphicalUpdate();
        void globalGraphicalUpdate();
        void setParentNotRendered();
        void setParentNotDrawn();
        void setChildrenDrawn();
        void setChildrenNotDrawn();
    };
} // namespace gui

#endif
