#include <stdint.h> // for uint16_t
#include <vector>

typedef uint16_t color_t;   // @Charles a remplacer quand tu auras mis la lib graphique
                            // Il faut aussi ajouter tous les consts

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

    uint16_t getX();
    uint16_t getY();
    uint16_t getWidth();
    uint16_t getHeight();

    uint16_t getAbsoluteX();
    uint16_t getAbsoluteY();

    void setBackgroundColor(const color_t color);
    void setBorderColor(const color_t color);

    color_t getBackgroundColor();
    color_t getBorderColor();

    virtual void onClick() {};
    virtual void onLongClick() {};
    virtual void onReleased() {};   // quand le widget est relaché correctement
    virtual void onNotClicked() {}; // quand le doit quitte simplement le widget
    virtual void onScroll() {};

    void enable();
    void disable();

    Widget* getMaster(); // get the highest parent
    Widget* getParent(); // get the parent

    void reloadAlone();  // mise a jour sur le buffer de l'objet
    void reloadParent();  // mise a jour qui concerne le buffer du parent
    void parentAreNotDrawn();       // fonction récurrente associée a reloadParent
    void parentAreNotRendered();    // fonction récurrente associée a reloadAlone
    void setChildrenDrawn();        // mettre tous les enfants en drawn=true

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
    bool m_isRendered;  // si le buffer est a jour
    bool m_isDrawn;     // si le widget est bien a jour sur l'écran
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

    static Widget* m_widgetPressed;   // si un widget est préssé sur l'écran (sinon nullptr)
};