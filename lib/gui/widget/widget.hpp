#include <stdint.h> // for uint16_t
#include <vector>

typedef uint16_t color_t; // @Charles a remplacer quand tu auras mis la lib graphique

namespace gui
{
    class Widget
    {
    public:
        Widget();
        virtual ~Widget();

        void renderAll();
        virtual void render() = 0;

        void updateAll();
        void update();

        void setX(uint16_t x);
        void setY(uint16_t y);
        void setWidth(uint16_t width);
        void setHeight(uint16_t height);

        [[nodiscard]] uint16_t getX() const;
        [[nodiscard]] uint16_t getY() const;
        [[nodiscard]] uint16_t getWidth() const;
        [[nodiscard]] uint16_t getHeight() const;

        [[nodiscard]] uint16_t getAbsoluteX() const;
        [[nodiscard]] uint16_t getAbsoluteY() const;

        void setBackgroundColor(color_t color);
        void setBorderColor(color_t color);

        [[nodiscard]] color_t getBackgroundColor() const;
        [[nodiscard]] color_t getBorderColor() const;

        virtual void onClick()
        {
        }

        virtual void onLongClick()
        {
        }

        /**
        * \brief When the widget in released correctly (Finger isn't leaving touch area)
        */
        virtual void onReleased()
        {
        }

        /**
        * \brief When finger leave the widget's touch area
        */
        virtual void onNotClicked()
        {
        }

        virtual void onScroll()
        {
        }

        void enable();
        void disable();

        /**
        * \brief Get the highest parent widget in the hierachy
        * \return the highest parent in the hierarchy
        */
        Widget* getMaster();

        /**
        * \brief Get the parent of this widget
        * \return the parent of this widget
        */
        [[nodiscard]] Widget* getParent() const;

        /**
        * Update the widget
        * \brief Deprecated, please use "Widget::reload()"
        */
        [[deprecated]] void reloadAlone(); // mise a jour locale

        /**
        * Update "itself" but on the parent side ?
        * \brief Deprecated, please use "Widget::getParent()::reload()"
        */
        [[deprecated]] void reloadParent();

        /**
        * \brief Error 404 : Documentation not found
        */
        void setChildrenDrawn(); // les enfants sont update sur l'écran

    private:
        // variables générales
        uint16_t m_x, m_y;
        uint16_t m_width, m_height;

        color_t m_backgroundColor;
        color_t m_borderColor;

        uint16_t m_borderSize;
        uint16_t m_borderRadius;

        Widget* m_parent;
        std::vector<Widget*> m_children;

        // variables sur les mouvements
        bool m_verticalScrollEnabled;
        bool m_horizontalScrollEnabled;

        uint16_t m_verticalScroll;
        uint16_t m_horizontalScroll;

        // variables de rendu
        bool m_isEnabled;


        bool m_isRendered; // si le buffer est a jour
        bool m_isDrawn; // si le widget est bien a jour sur l'écran
        static Widget* masterOfRender;

        // variables sur les events
        enum PressedState
        {
            NOT_PRESSED,
            PRESSED,
            SLIDED,
            RELEASED
        };

        PressedState m_pressedState;
        bool m_isScrolling;

        static Widget* m_widgetPressed; // si un widget est préssé sur l'écran (sinon nullptr)
    };
}
