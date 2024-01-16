#include "widget.hpp"

Widget* Widget::m_widgetPressed = nullptr;
Widget* Widget::masterOfRender = nullptr;

Widget::Widget() :
    m_x(0), m_y(0),
    m_width(0), m_height(0),
    m_backgroundColor(0),
    m_borderColor(0),
    m_borderSize(0),
    m_borderRadius(0),
    m_parent(nullptr),
    m_verticalScrollEnabled(false),
    m_horizontalScrollEnabled(false),
    m_verticalScroll(0),
    m_horizontalScroll(0),
    m_isEnabled(true),
    m_isRendered(false),
    m_isDrawn(false),
    m_pressedState(NOT_PRESSED),
    m_isScrolling(false)
{
    // Initialiser d'autres membres si nécessaire dans le constructeur
}

Widget::~Widget()
{
    // Libération de la mémoire allouée pour les enfants de l'objet
    for(int i = 0; i < m_children.size(); i++)
    {
        if(m_children[i] != nullptr)
        {
            delete m_children[i];
        }
    }
}

void Widget::renderAll()
{
    if (!m_isEnabled)
        return;

    if (!m_isRendered)
    {
        // initialiser le buffer ou le clear

        render();

        for (auto child : m_children)
        {
            // restreindre l'écriture de child sur le buffer local a ses coordonées
            child->renderAll();

            m_isRendered = true;
        }
    }

    if (!m_isDrawn || (m_parent!=nullptr && m_parent->m_isRendered==false))
    {
        if(m_parent != nullptr && m_parent->m_isDrawn == false)  // le parent demande le rendu
        {
            // push le buffer local vers le buffer du parent
        }else   // le parent ne demande pas de rendu ou le parent n'existe pas
        {
            // restreindre l'écriture sur l'écran en fonction du buffer local

            // push le buffer local vers l'écran

            setChildrenDrawn();
        }
    }
}

void Widget::updateAll()
{
    for (auto child : m_children)
    {
        child->updateAll();
    }
}

void Widget::update()
{
    // algorithme de mise a jour des interactions tactiles
}

void Widget::setX(uint16_t x)
{
    m_x = x;
}

void Widget::setY(uint16_t y)
{
    m_y = y;
}

void Widget::setWidth(uint16_t width)
{
    m_width = width;
}

void Widget::setHeight(uint16_t height)
{
    m_height = height;
}

uint16_t Widget::getAbsoluteX()
{
    if (m_parent == nullptr)
        return getX();
    else
        return m_parent->getAbsoluteX() + getX();
}

uint16_t Widget::getAbsoluteY()
{
    if (m_parent == nullptr)
        return getY();
    else
        return m_parent->getAbsoluteY() + getY();
}

uint16_t Widget::getX()
{
    return m_x;
}

uint16_t Widget::getY()
{
    return m_y;
}

uint16_t Widget::getWidth()
{
    return m_width;
}

uint16_t Widget::getHeight()
{
    return m_height;
}

void Widget::setBackgroundColor(const color_t color)
{
    m_backgroundColor = color;
}

void Widget::setBorderColor(const color_t color)
{
    m_borderColor = color;
}

color_t Widget::getBackgroundColor()
{
    return m_backgroundColor;
}

color_t Widget::getBorderColor()
{
    return m_borderColor;
}

void Widget::enable()
{
    m_isEnabled = true;
}

void Widget::disable()
{
    m_isEnabled = false;
}

Widget* Widget::getMaster()
{
    Widget* master = this;

    if (this->m_parent != nullptr)
    {
        master = m_parent->getMaster();
    }

    return master;
}

Widget* Widget::getParent()
{
    return m_parent;
}

void Widget::reloadAlone()
{
    this->m_isDrawn = false;
    this->m_isRendered = false;

    if(m_parent != nullptr)
        m_parent->parentAreNotRendered();
}

void Widget::reloadParent()
{
    this->m_isDrawn = false;
    this->m_isRendered = false;

    if(m_parent != nullptr)
        m_parent->parentAreNotDrawn();
}

void Widget::parentAreNotRendered()
{
    if(m_parent != nullptr)
        m_parent->parentAreNotRendered();

    m_isRendered = false;
}

void Widget::parentAreNotDrawn()
{
    if(m_parent != nullptr)
        m_parent->parentAreNotDrawn();

    m_isDrawn = false;
    m_isRendered = false;
}

void Widget::childrenAreDrawn()
{
    for (int i = 0; i < m_children.size(); i++)
    {
        if(m_children[i] != nullptr)
            m_children[i]->childrenAreDrawn();
    }

    m_isDrawn = true;
}