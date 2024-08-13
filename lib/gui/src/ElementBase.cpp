#include "ElementBase.hpp"
#include <graphics.hpp>

#include "gui.hpp"
#include <threads.hpp>

#include <iostream>

// TODO : Remove this, the user need to define its widget for the screen itself.
gui::ElementBase *gui::ElementBase::widgetPressed = nullptr;
gui::ElementBase *gui::ElementBase::masterOfRender = nullptr;
gui::ElementBase *gui::ElementBase::mainWindow = nullptr;
int16_t gui::ElementBase::touchX, gui::ElementBase::touchY = -1;
int16_t gui::ElementBase::originTouchX, gui::ElementBase::originTouchY = -1;
int16_t gui::ElementBase::m_lastTouchX, gui::ElementBase::m_lastTouchY;
int16_t gui::ElementBase::lastEventTouchX, gui::ElementBase::lastEventTouchY;
bool gui::ElementBase::scrolling = false;
gui::ElementBase::PressedState gui::ElementBase::globalPressedState = gui::ElementBase::PressedState::NOT_PRESSED;

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

void gui::ElementBase::renderAll(bool onScreen)
{
    if(!isInside())
        return;

    if (!m_isEnabled)
        return;

    if (!m_isRendered)
    {
        StandbyMode::triggerPower();
        // initialiser le buffer ou le clear
        if(m_surface != nullptr && (m_surface->getWidth() != this->getWidth() || m_surface->getHeight() != this->getHeight()))
            m_surface = nullptr;

        if (m_surface == nullptr)
            m_surface = std::make_shared<graphics::Surface>(m_width, m_height);

        render();

        for (const auto child : m_children)
        {
            child->renderAll(false);
        }

        m_isRendered = true;
    }

    if (!m_isDrawn || (m_parent != nullptr && m_parent->m_isRendered == false))
    {
        StandbyMode::triggerPower();
        if (!onScreen) // le parent demande le rendu
        {
            m_parent->m_surface->pushSurface(m_surface.get(), getX(), getY());
        }
        else // le parent ne demande pas de rendu ou le parent n'existe pas
        {
            graphics::setWindow(getAbsoluteX(), getAbsoluteY(), getWidth(), getHeight());
            graphics::showSurface(m_surface.get(), getAbsoluteX(), getAbsoluteY());
            graphics::setWindow();

            setChildrenDrawn();
        }
    }
}


bool gui::ElementBase::updateAll()
{
    if(!isInside())
    {
        if(m_surface != nullptr)
            free();
        return false;
    }

    if (m_parent == nullptr)
    {
        StandbyMode::wait();
        if(mainWindow != this)
        {
            mainWindow = this;
            this->m_isDrawn = false;
        }

        graphics::getTouchPos(&touchX, &touchY);
    }

    if (!m_isDrawn)
        renderAll();

    bool returnV = false;

    for (auto child : m_children)
    {
        if (child->updateAll())
        {
            returnV = true; // if child had an event, ignore local events
            break;
        }
    }

    update();
    
    
    if(this->m_parent == nullptr)
        graphics::touchIsRead();

    return returnV;
}

gui::ElementBase* gui::ElementBase::getHigestXScrollableParent()
{
    if(m_horizontalScrollEnabled)
    {
        return this;
    }
    else
    {
        if(m_parent != nullptr)
            return m_parent->getHigestXScrollableParent();
        else
            return nullptr;
    }
}

gui::ElementBase* gui::ElementBase::getHigestYScrollableParent()
{
    if(m_verticalScrollEnabled)
    {
        return this;
    }
    else
    {
        if(m_parent != nullptr)
            return m_parent->getHigestYScrollableParent();
        else
            return nullptr;
    }
}

bool gui::ElementBase::update()
{
    // algorithme de mise a jour des interactions tactiles

    widgetUpdate();

    if (!m_hasEvents && widgetPressed != this)
        return false;


    if (widgetPressed != nullptr && widgetPressed != this)
        return false;

    bool isScreenTouched = graphics::isTouched();
    bool isWidgetTouched = isScreenTouched && (getAbsoluteX()-10 < touchX && touchX < getAbsoluteX() + getWidth() +10 &&
                            getAbsoluteY()-10 < touchY && touchY < getAbsoluteY() + getHeight() +10);

    bool returnValue = false;

    //std::cout << "globalPressedState: " << globalPressedState << std::endl;
    //std::cout << "widgetPressed: " << int(widgetPressed != nullptr) << std::endl;

    if (isScreenTouched)
    {
        if (isWidgetTouched && globalPressedState == NOT_PRESSED)
        {
            globalPressedState = PRESSED;
            widgetPressed = this;

            originTouchX = touchX;
            originTouchY = touchY;

            m_lastTouchX = touchX;
            m_lastTouchY = touchY;

            returnValue = true;
        }
        else if (widgetPressed == this)
        {
            bool isScrollingX = abs(m_lastTouchX - touchX) > SCROLL_STEP;
            bool isScrollingY = abs(m_lastTouchY - touchY) > SCROLL_STEP;
            bool isScrolling = isScrollingX || isScrollingY;
    
            if(isScrollingX)
            {
                gui::ElementBase* nearScrollableObject = getHigestXScrollableParent();
                globalPressedState = SCROLLX;

                if(nearScrollableObject == this)
                {
                    while(m_lastTouchX + SCROLL_STEP < touchX)
                    {
                        onScrollRight();
                        m_lastTouchY += SCROLL_STEP;
                    }
                    while(m_lastTouchX - SCROLL_STEP > touchX)
                    {
                        onScrollLeft();
                        m_lastTouchY -= SCROLL_STEP;
                    }
                }
                else if(nearScrollableObject == nullptr)
                {
                    globalPressedState = LOCKED;
                }
                else
                {
                    widgetPressed = getHigestXScrollableParent();
                }

                globalPressedState = SCROLLX;
                m_lastTouchX = touchX;

                returnValue = true;
            }

            if(isScrollingY)
            {
                gui::ElementBase* nearScrollableObject = getHigestYScrollableParent();
                globalPressedState = SCROLLY;

                if(nearScrollableObject == this)
                {
                    while(m_lastTouchY + SCROLL_STEP < touchY)
                    {
                        onScrollUp();
                        m_lastTouchY += SCROLL_STEP;
                    }
                    while(m_lastTouchY - SCROLL_STEP > touchY)
                    {
                        onScrollDown();
                        m_lastTouchY -= SCROLL_STEP;
                    }
                }
                else if(nearScrollableObject == nullptr)
                {
                    globalPressedState = LOCKED;
                }
                else
                {
                    widgetPressed = getHigestYScrollableParent();
                }

                globalPressedState = SCROLLY;
                m_lastTouchY = touchY;

                returnValue = true;
            }
        }
    }
    else
    {
        if(globalPressedState == PRESSED)
        {
            this->m_pressedState = RELEASED;
            lastEventTouchX = originTouchX;
            lastEventTouchY = originTouchY;
            onReleased();
        }
        
        globalPressedState = NOT_PRESSED;
        widgetPressed = nullptr;

        returnValue = true;
    }

    return returnValue;
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

int16_t gui::ElementBase::getAbsoluteX() const
{
    if (m_parent == nullptr)
        return getX();

    return m_parent->getAbsoluteX() + getX();
}

int16_t gui::ElementBase::getAbsoluteY() const
{
    if (m_parent == nullptr)
        return getY();

    return m_parent->getAbsoluteY() + getY();
}

int16_t gui::ElementBase::getX() const
{
    return (int) m_x/* + (m_parent!=nullptr)?(m_parent->m_horizontalScroll):(0)*/;
}

int16_t gui::ElementBase::getY() const
{
    if(m_parent!=nullptr)
    {
        return m_y - m_parent->m_verticalScroll;
    }
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

bool gui::ElementBase::isFocused(bool forced)
{
    if(forced)
    {
        return (touchX != -1 && touchY != -1 && getAbsoluteX()-10 < touchX && touchX < getAbsoluteX() + getWidth() +10 && // l'objet est touché
            getAbsoluteY()-10 < touchY && touchY < getAbsoluteY() + getHeight() +10);
    }
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

bool gui::ElementBase::getIsEnabled() const
{
    return m_isEnabled;
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

std::shared_ptr<graphics::Surface> gui::ElementBase::getAndSetSurface()
{
    if (this->m_surface == nullptr)
    {
        this->m_surface = std::make_shared<graphics::Surface>(this->m_width, this->m_height);
    }
    return this->m_surface;
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

void gui::ElementBase::setChildrenNotDrawn()
{
    m_isDrawn = false;

    for (int i = 0; i < m_children.size(); i++) // dire aux enfants qu'il sont actualisés sur l'écran
    {
        if (m_children[i] != nullptr)
            m_children[i]->m_isDrawn = false;
    }
}

/**
 * "Returns" the <b>absolute</b> last touched position.
 * @param x The pointer to assign the x value to.
 * @param y The pointer to assign the y value to.
 * @see gui::ElementBase::getLastTouchPosRel
 * @see {gui}
 */
void gui::ElementBase::getLastTouchPosAbs(int16_t* x, int16_t* y) const
{
    *x = m_lastTouchX;
    *y = m_lastTouchY;
}

/**
 * "Returns" the <b>relative</b> last touched position.
 * @param x The pointer to assign the x value to.
 * @param y The pointer to assign the y value to.
 * @see gui::ElementBase::getLastTouchPos
 * @see {gui}
 */
void gui::ElementBase::getLastTouchPosRel(int16_t* x, int16_t* y) const
{
    *x = lastEventTouchX - getAbsoluteX();
    *y = lastEventTouchY - getAbsoluteY();
}

void gui::ElementBase::free()
{
    if(m_surface != nullptr)
        m_surface.reset();

    setParentNotRendered();
    
    for (auto child : m_children)
    {
        child->free();
    }
}

bool gui::ElementBase::isInside()
{
    if(m_parent == nullptr)
        return true;

    if(getX() + getWidth() < 0)
        return false;
    if(getY() + getHeight() < 0)
        return false;
    if(getX() > m_parent->getWidth())
        return false;
    if(getY() > m_parent->getHeight())
        return false;

    return true;
}