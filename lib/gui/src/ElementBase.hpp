#ifndef ELEMENTBASE_HPP
#define ELEMENTBASE_HPP

#include <cstdint> // for uint16_t
#include <Surface.hpp>
#include <vector>

typedef uint16_t color_t; // @Charles a remplacer quand tu auras mis la lib graphique

namespace gui
{
    class ElementBase
    {
    public:
        ElementBase();
        virtual ~ElementBase();

        virtual void render() = 0;
        void renderAll();

        bool updateAll();
        bool update();

        void setX(uint16_t x);
        void setY(uint16_t y);
        void setWidth(uint16_t width);
        void setHeight(uint16_t height);

        uint16_t getX() const;
        uint16_t getY() const;
        uint16_t getWidth() const;
        uint16_t getHeight() const;

        uint16_t getAbsoluteX() const;
        uint16_t getAbsoluteY() const;

        void setBackgroundColor(color_t color);
        void setBorderColor(color_t color);

        void setRadius(uint16_t r);
        uint16_t getRadius() const;

        void setBorderSize(uint16_t size);
        uint16_t getBorderSize() const;

        color_t getBackgroundColor() const;
        color_t getBorderColor() const;

        bool isTouched(); // retourne si le widget a été pressé puis relaché (nb, l'appel de la fonction annule m'état précédent)
        bool isFocused(); // retourne si le doigt est sur le widget

        virtual void onClick() {}
        virtual void onLongClick() {}

        /**
         * \brief When the widget is no longer considered as touched even if the finger is still on the screen
         */
        virtual void onReleased() {}

        /**
         * \brief When finger leave the screen
         */
        virtual void onNotClicked() {}

        virtual void onScroll() {}

        void enable();
        void disable();

        /**
         * \brief Get the highest parent widget in the hierachy
         * \return the highest parent in the hierarchy
         */
        ElementBase *getMaster();
        ElementBase *getParent() const;
        void addChild(ElementBase *child);

    protected:
        // variables générales
        uint16_t m_x, m_y;
        uint16_t m_width, m_height;

        color_t m_backgroundColor;
        color_t m_borderColor;

        uint16_t m_borderSize;
        uint16_t m_borderRadius;

        ElementBase *m_parent;
        std::vector<ElementBase *> m_children;

        // variables sur les mouvements
        bool m_verticalScrollEnabled;
        bool m_horizontalScrollEnabled;

        uint16_t m_verticalScroll;
        uint16_t m_horizontalScroll;

        // variables de rendu
        bool m_isEnabled;
        bool m_hasEvents; // si l'objet est réactif aux events tactiles

        bool m_isRendered; // si le buffer est a jour
        bool m_isDrawn;    // si le widget est bien a jour sur l'écran
        static ElementBase *masterOfRender;

        // variables sur les events
        enum PressedState
        {
            NOT_PRESSED,
            PRESSED,
            SLIDED,
            RELEASED
        };

        PressedState m_pressedState;

        static ElementBase *m_widgetPressed; // si un widget est préssé sur l'écran (sinon nullptr)
        static int16_t originTouchX, originTouchY;
        static int16_t touchX, touchY;

        std::shared_ptr<graphics::Surface> m_surface; // Surface to render the widget

    private:
        void localGraphicalUpdate();
        void globalGraphicalUpdate();
        void setParentNotRendered();
        void setParentNotDrawn();
        void setChildrenDrawn();
    };
}

#endif