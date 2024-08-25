#ifndef ELEMENTBASE_HPP
#define ELEMENTBASE_HPP

#include <cstdint> // for uint16_t
#include <Surface.hpp>
#include <vector>

typedef uint16_t color_t; // @Charles a remplacer quand tu auras mis la lib graphique
#define SCROLL_STEP 20

namespace gui
{
    class ElementBase
    {
    public:
        ElementBase();
        virtual ~ElementBase();

        virtual void render() = 0;
        void renderAll(bool onScreen = true);

        bool updateAll();
        bool update();
        virtual void widgetUpdate() {};

        void setX(uint16_t x);
        void setY(uint16_t y);
        void setWidth(uint16_t width);
        void setHeight(uint16_t height);

        int16_t getX() const;
        int16_t getY() const;
        uint16_t getWidth() const;
        uint16_t getHeight() const;

        int16_t getAbsoluteX() const;
        int16_t getAbsoluteY() const;

        void setBackgroundColor(color_t color);
        void setBorderColor(color_t color);

        void setRadius(uint16_t r);
        uint16_t getRadius() const;

        void setBorderSize(uint16_t size);
        uint16_t getBorderSize() const;

        color_t getBackgroundColor() const;
        color_t getBorderColor() const;

        bool isTouched(); // retourne si le widget a été pressé puis relaché (nb, l'appel de la fonction annule m'état précédent)
        bool isFocused(bool forced = false); // retourne si le doigt est sur le widget

        virtual void onClick() {}
        virtual void onLongClick() {}

        /**
         * \brief When finger leave the widget ans is considered as released
         */
        virtual void onReleased() {}

        
        /**
         * \brief When the widget is no longer considered as touched even if the finger is still on the screen
         */virtual void onNotClicked() {}

        virtual void onScroll() {}

        void enable();
        void disable();
        bool getIsEnabled() const;

        void free();    // free the buffers in the ram to allow more windows to work at the same time

        /**
         * \brief Get the highest parent widget in the hierachy
         * \return the highest parent in the hierarchy
         */
        ElementBase *getMaster();
        ElementBase *getParent() const;
        void addChild(ElementBase *child);

        ElementBase *m_parent;
        std::vector<ElementBase *> m_children;
        static int16_t touchX, touchY;
        static int16_t lastEventTouchX, lastEventTouchY;

        uint16_t m_x, m_y;
    protected:
        // variables générales
        uint16_t m_width, m_height;

        color_t m_backgroundColor;
        color_t m_borderColor;

        uint16_t m_borderSize;
        uint16_t m_borderRadius;


        // variables sur les mouvements
        bool m_verticalScrollEnabled;
        bool m_horizontalScrollEnabled;
        static bool scrolling;

        int16_t m_verticalScroll = 0;
        int16_t m_horizontalScroll = 0;

        // variables de rendu
        bool m_isEnabled;
        bool m_hasEvents; // si l'objet est réactif aux events tactiles

        bool m_isRendered; // si le buffer est a jour
        bool m_isDrawn;    // si le widget est bien a jour sur l'écran
        bool m_autoDelete; // si le buffer du widget est supprimé automatiquement si en dehors de l'écran

        static ElementBase *masterOfRender;
        static ElementBase *mainWindow;

        // variables sur les events
        enum PressedState
        {
            NOT_PRESSED,
            PRESSED,
            SLIDED,
            RELEASED,
            SCROLLX,
            SCROLLY,
            LOCKED
        };

        PressedState m_pressedState;

        static PressedState globalPressedState;
        static ElementBase *widgetPressed; // si un widget est préssé sur l'écran (sinon nullptr)
        static int16_t originTouchX, originTouchY;

        ElementBase* getHigestXScrollableParent();
        ElementBase* getHigestYScrollableParent();

        bool isInside(); // si le widget est visible dans son parent

        static int16_t m_lastTouchX, m_lastTouchY;
        void getLastTouchPosAbs(int16_t* x, int16_t* y) const;
        void getLastTouchPosRel(int16_t* x, int16_t* y) const;

        virtual void onScrollUp() {};
        virtual void onScrollDown() {};
        virtual void onScrollLeft() {};
        virtual void onScrollRight() {};

        std::shared_ptr<graphics::Surface> m_surface; // Surface to render the widget
        std::shared_ptr<graphics::Surface> getAndSetSurface(); // Get the m_surface of the the ElementBase and initialize it if it is nullptr
    protected:
        void localGraphicalUpdate();
        void globalGraphicalUpdate();
        void setParentNotRendered();
        void setParentNotDrawn();
        void setChildrenDrawn();
        void setChildrenNotDrawn();
    };
}

#endif