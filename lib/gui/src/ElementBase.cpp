#include "ElementBase.hpp"
#include <graphics.hpp>

#include "gui.hpp"

// TODO : Remove this, the user need to define its widget for the screen itself.
gui::ElementBase* gui::ElementBase::m_widgetPressed = nullptr;
gui::ElementBase* gui::ElementBase::masterOfRender = nullptr;

gui::ElementBase::ElementBase() :
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

gui::ElementBase::~ElementBase()
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

void gui::ElementBase::renderAll()
{
    if (!m_isEnabled)
        return;

    if (!m_isRendered)
    {
        // initialiser le buffer ou le clear

        if (m_surface == nullptr)
        {
            m_surface = std::make_shared<graphics::Surface>(m_width, m_height);
        }
        else
        {
            m_surface->clear();
        }

        render();

        for (const auto child : m_children)
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

            // TODO : Change position
            m_parent->m_surface->pushSurface(m_surface.get(), m_x, m_y);
        }
        else   // le parent ne demande pas de rendu ou le parent n'existe pas
        {
            // restreindre l'écriture sur l'écran en fonction du buffer local

            // push le buffer local vers l'écran
            // TODO : Change position
            graphics::showSurface(m_surface.get(), getAbsoluteX(), getAbsoluteY());

            setChildrenDrawn();
        }
    }
}

void gui::ElementBase::updateAll()
{
    for (auto child : m_children)
    {
        child->updateAll();
    }
}

void gui::ElementBase::update()
{
    // algorithme de mise a jour des interactions tactiles
}

void gui::ElementBase::setX(uint16_t x)
{
    m_x = x;
}

void gui::ElementBase::setY(uint16_t y)
{
    m_y = y;
}

void gui::ElementBase::setWidth(uint16_t width)
{
    m_width = width;
}

void gui::ElementBase::setHeight(uint16_t height)
{
    m_height = height;
}

uint16_t gui::ElementBase::getAbsoluteX() const
{
    if (m_parent == nullptr)
        return getX();

    return m_parent->getAbsoluteX() + getX();
}

uint16_t gui::ElementBase::getAbsoluteY() const
{
    if (m_parent == nullptr)
        return getY();

    return m_parent->getAbsoluteY() + getY();
}

uint16_t gui::ElementBase::getX() const
{
    return m_x;
}

uint16_t gui::ElementBase::getY() const
{
    return m_y;
}

uint16_t gui::ElementBase::getWidth() const
{
    return m_width;
}

uint16_t gui::ElementBase::getHeight() const
{
    return m_height;
}

void gui::ElementBase::setBackgroundColor(const color_t color)
{
    m_backgroundColor = color;
}

void gui::ElementBase::setBorderColor(const color_t color)
{
    m_borderColor = color;
}

color_t gui::ElementBase::getBackgroundColor() const
{
    return m_backgroundColor;
}

color_t gui::ElementBase::getBorderColor() const
{
    return m_borderColor;
}

void gui::ElementBase::enable()
{
    m_isEnabled = true;
}

void gui::ElementBase::disable()
{
    m_isEnabled = false;
}

gui::ElementBase* gui::ElementBase::getMaster()
{
    // We shoud probably remove this function.
    // Because the "Master" widget (and rename it to "main" please)
    // Is for almost every cases the "Screen" widget.
    // So the user already have a reference to it.

    ElementBase* master = this;

    if (this->m_parent != nullptr)
    {
        master = m_parent->getMaster();
    }

    return master;
}

gui::ElementBase* gui::ElementBase::getParent() const
{
    return m_parent;
}

void gui::ElementBase::addChild(gui::ElementBase* child)
{
    m_children.push_back(child);
}

void gui::ElementBase::localGraphicalUpdate()
{
    this->m_isDrawn = false;
    this->m_isRendered = false;

    if(m_parent != nullptr)
        setParentNotRendered();
}

void gui::ElementBase::globalGraphicalUpdate()
{
    this->m_isDrawn = false;
    this->m_isRendered = false;

    if(this->m_parent != nullptr)
        setParentNotDrawn();
}

void gui::ElementBase::setParentNotRendered()
{
    if(m_parent != nullptr)
        setParentNotRendered();
    
    this->m_isRendered = false;
}

void gui::ElementBase::setParentNotDrawn()
{
    if(m_parent != nullptr)
        setParentNotDrawn();

    this->m_isDrawn = false;
    this->m_isRendered = false;
}

void gui::ElementBase::setChildrenDrawn()
{
    m_isDrawn = true;

    for (int i = 0; i < m_children.size(); i++) // dire aux enfants qu'il sont actualisés sur l'écran
    {
        if(m_children[i] != nullptr)
            m_children[i]->setChildrenDrawn();
    }
}