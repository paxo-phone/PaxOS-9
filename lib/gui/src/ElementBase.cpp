#include "ElementBase.hpp"
#include <graphics.hpp>

#include "gui.hpp"

#include <iostream>

// TODO : Remove this, the user need to define its widget for the screen itself.
gui::ElementBase *gui::ElementBase::m_widgetPressed = nullptr;
gui::ElementBase *gui::ElementBase::masterOfRender = nullptr;
int16_t gui::ElementBase::touchX, gui::ElementBase::touchY = -1;
int16_t gui::ElementBase::originTouchX, gui::ElementBase::originTouchY = -1;

gui::ElementBase::ElementBase() : m_x(0), m_y(0),
                                  m_width(0), m_height(0),
                                  m_backgroundColor(COLOR_WHITE),
                                  m_borderColor(COLOR_WHITE),
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
                                  m_hasEvents(false)
{
    // Initialiser d'autres membres si nécessaire dans le constructeur
}

gui::ElementBase::~ElementBase()
{
    // Libération de la mémoire allouée pour les enfants de l'objet
    for (int i = 0; i < m_children.size(); i++)
    {
        if (m_children[i] != nullptr)
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
        if(m_surface != nullptr && (m_surface->getWidth() != this->getWidth() || m_surface->getHeight() != this->getHeight()))
            m_surface = nullptr;

        if (m_surface == nullptr)
        {
            m_surface = std::make_shared<graphics::Surface>(m_width, m_height);
            //m_surface->clear(COLOR_WHITE);
        }
        else
        {
            //m_surface->clear(COLOR_WHITE);
        }

        render();

        for (const auto child : m_children)
        {
            // restreindre l'écriture de child sur le buffer local a ses coordonées
            child->renderAll();
        }

        m_isRendered = true;
    }

    if (!m_isDrawn || (m_parent != nullptr && m_parent->m_isRendered == false))
    {
        if (m_parent != nullptr && m_parent->m_isDrawn == false) // le parent demande le rendu
        {
            // push le buffer local vers le buffer du parent
            // TODO : Change position
            m_parent->m_surface->pushSurface(m_surface.get(), m_x, m_y);
        }
        else // le parent ne demande pas de rendu ou le parent n'existe pas
        {
            // restreindre l'écriture sur l'écran en fonction du buffer local

            // push le buffer local vers l'écran
            // TODO : Change position
            graphics::showSurface(m_surface.get(), getAbsoluteX(), getAbsoluteY());
            setChildrenDrawn();
        }
    }
}

bool gui::ElementBase::updateAll()
{
    if (m_parent == nullptr)
    {
        // todo: update events

        graphics::getTouchPos(&touchX, &touchY);
    }

    if (!m_isDrawn)
        renderAll();

    for (auto child : m_children)
    {
        if (child->updateAll())
        {
            return true; // if child had an event, ignore local events
        }
    }

    return update();
}

bool gui::ElementBase::update()
{
    // algorithme de mise a jour des interactions tactiles

    if (!m_hasEvents)
        return false;

    widgetUpdate();

    if (m_widgetPressed != nullptr && m_widgetPressed != this)
        return false;

    // check if the finger is currently on the widget
    if (touchX != -1 && touchY != -1)
    {
        if (getAbsoluteX()-10 < touchX && touchX < getAbsoluteX() + getWidth() +10 && // l'objet est touché
            getAbsoluteY()-10 < touchY && touchY < getAbsoluteY() + getHeight() +10)
        {
            if (m_widgetPressed == nullptr && m_pressedState == PressedState::NOT_PRESSED) // l'objet est touché pour la première fois
            {
                m_widgetPressed = this;
                m_pressedState = PressedState::PRESSED;

                originTouchX = touchX; // sauvegarder les coordonnées du premier point (pour le scroll)
                originTouchY = touchY;

                return true;
            }
        }
        else if (m_pressedState == PressedState::NOT_PRESSED)
        {
            return false;
        }

        // scroll
        if (abs(originTouchX - touchX) > 5) // todo: ajout d'une constante de seuil
        {
            m_pressedState = PressedState::NOT_PRESSED;
            return true;
        }

        if (abs(originTouchY - touchY) > 5)
        {
            m_pressedState = PressedState::NOT_PRESSED;
            return true;
        }
    }
    else
    {
        originTouchX = -1;
        originTouchY = -1;
    }

    if (touchX == -1 && touchY == -1 && m_widgetPressed == this && m_pressedState != PressedState::RELEASED)
    {
        if (m_pressedState == PressedState::PRESSED)
        {
            m_pressedState = PressedState::RELEASED;
            onReleased();
        }

        m_widgetPressed = nullptr;
    }

    return false;
}

void gui::ElementBase::setX(uint16_t x)
{
    m_x = x;
    globalGraphicalUpdate();
}

void gui::ElementBase::setY(uint16_t y)
{
    m_y = y;
    globalGraphicalUpdate();
}

void gui::ElementBase::setWidth(uint16_t width)
{
    m_width = width;
    globalGraphicalUpdate();
}

void gui::ElementBase::setHeight(uint16_t height)
{
    m_height = height;
    globalGraphicalUpdate();
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
    localGraphicalUpdate();
}

void gui::ElementBase::setBorderColor(const color_t color)
{
    m_borderColor = color;
    localGraphicalUpdate();
}

void gui::ElementBase::setRadius(uint16_t r)
{
    this->m_borderRadius = r;
}

uint16_t gui::ElementBase::getRadius() const
{
    return m_borderRadius;
}

void gui::ElementBase::setBorderSize(uint16_t size)
{
    this->m_borderSize = size;
}

uint16_t gui::ElementBase::getBorderSize() const
{
    return m_borderSize;
}

color_t gui::ElementBase::getBackgroundColor() const
{
    return m_backgroundColor;
}

color_t gui::ElementBase::getBorderColor() const
{
    return m_borderColor;
}

bool gui::ElementBase::isTouched()
{
    m_hasEvents = true;
    if (m_pressedState == PressedState::RELEASED)
    {
        m_pressedState = PressedState::NOT_PRESSED;
        return true;
    }
    return false;
}

bool gui::ElementBase::isFocused()
{
    m_hasEvents = true;
    return m_pressedState == PressedState::PRESSED;
}

void gui::ElementBase::enable()
{
    m_isEnabled = true;
    globalGraphicalUpdate();
}

void gui::ElementBase::disable()
{
    m_isEnabled = false;
    globalGraphicalUpdate();
}

gui::ElementBase *gui::ElementBase::getMaster()
{
    // We shoud probably remove this function.
    // Because the "Master" widget (and rename it to "main" please)
    // Is for almost every cases the "Screen" widget.
    // So the user already have a reference to it.

    ElementBase *master = this;

    if (this->m_parent != nullptr)
    {
        master = m_parent->getMaster();
    }

    return master;
}

gui::ElementBase *gui::ElementBase::getParent() const
{
    return m_parent;
}

void gui::ElementBase::addChild(gui::ElementBase *child)
{
    m_children.push_back(child);
    child->m_parent = this;
}

void gui::ElementBase::localGraphicalUpdate()
{
    this->m_isDrawn = false;
    this->m_isRendered = false;

    if (m_parent != nullptr)
        setParentNotRendered();
}

void gui::ElementBase::globalGraphicalUpdate()
{
    this->m_isDrawn = false;
    this->m_isRendered = false;

    if (this->m_parent != nullptr)
        setParentNotDrawn();
}

void gui::ElementBase::setParentNotRendered()
{
    if (m_parent != nullptr)
        m_parent->setParentNotRendered();

    this->m_isRendered = false;
}

void gui::ElementBase::setParentNotDrawn()
{
    if (m_parent != nullptr)
        m_parent->setParentNotDrawn();

    this->m_isDrawn = false;
    this->m_isRendered = false;
}

void gui::ElementBase::setChildrenDrawn()
{
    m_isDrawn = true;

    for (int i = 0; i < m_children.size(); i++) // dire aux enfants qu'il sont actualisés sur l'écran
    {
        if (m_children[i] != nullptr)
            m_children[i]->setChildrenDrawn();
    }
}