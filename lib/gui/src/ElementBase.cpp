#include "ElementBase.hpp"

#include "gui.hpp"

#include <graphics.hpp>
#include <iostream>
#include <libsystem.hpp>
#include <standby.hpp>

// TODO : Remove this, the user need to define its widget for the screen itself.
gui::ElementBase* gui::ElementBase::widgetPressed = nullptr;
gui::ElementBase* gui::ElementBase::masterOfRender = nullptr;
gui::ElementBase* gui::ElementBase::mainWindow = nullptr;
int16_t gui::ElementBase::touchX, gui::ElementBase::touchY = -1;
int16_t gui::ElementBase::originTouchX, gui::ElementBase::originTouchY = -1;
int16_t gui::ElementBase::lastTouchX, gui::ElementBase::lastTouchY;
int16_t gui::ElementBase::lastEventTouchX, gui::ElementBase::lastEventTouchY;
bool gui::ElementBase::scrolling = false;
bool gui::ElementBase::wasPressed = false; // if the screen was pressed in the last update
gui::ElementBase::PressedState gui::ElementBase::globalPressedState = NOT_PRESSED;

void gui::ElementBase::resetStates()
{
    widgetPressed = nullptr;
    mainWindow = nullptr;
    touchX = -1;
    touchY = -1;
    originTouchX = -1;
    originTouchY = -1;
    lastTouchX = -1;
    lastTouchY = -1;
    lastEventTouchX = -1;
    lastEventTouchY = -1;
    scrolling = false;
    globalPressedState = NOT_PRESSED;
}

gui::ElementBase::ElementBase() :
    x_(0), y_(0), width_(0), height_(0), backgroundColor_(COLOR_WHITE),
    borderColor_(COLOR_WHITE), borderSize_(0), borderRadius_(0), parent_(nullptr),
    verticalScrollEnabled_(false), horizontalScrollEnabled_(false), verticalScroll_(0),
    horizontalScroll_(0), isEnabled_(true), isRendered_(false), isDrawn_(false),
    pressedState_(NOT_PRESSED), hasEvents_(false)
{
    // Initialiser d'autres membres si nécessaire dans le constructeur
}

gui::ElementBase::~ElementBase()
{

    // force le rafraichisseent sur un delete d'un enfant
    if (parent_ != nullptr)
        parent_->localGraphicalUpdate();

    // Libération de la mémoire allouée pour les enfants de l'objet
    for (int i = 0; i < children_.size(); i++)
        if (children_[i] != nullptr)
            delete children_[i];

    if (widgetPressed == this)
    {
        widgetPressed = nullptr;
        globalPressedState = PressedState::NOT_PRESSED;
    }
}

void gui::ElementBase::renderAll(bool onScreen)
{
    if (!isInside())
        return;

    if (!isEnabled_)
        return;

    if (!isRendered_)
    {
        StandbyMode::triggerPower();

        // Use it to initialize data
        preRender();

        // initialiser le buffer ou le clear
        if (surface_ != nullptr && (surface_->getWidth() != this->getWidth() ||
                                     surface_->getHeight() != this->getHeight()))
        {
            surface_ = nullptr;
        }

        if (surface_ == nullptr)
        {
            freeRamFor(width_ * height_, this->getMaster());
            surface_ = std::make_shared<graphics::Surface>(width_, height_);
        }

        // Render the element
        render();

        for (const auto child : children_) child->renderAll(false);

        isRendered_ = true;

        // Use it to clear data
        postRender();
    }

    if (!isDrawn_ || (parent_ != nullptr && parent_->isRendered_ == false))
    {
        StandbyMode::triggerPower();

        if (!onScreen) // le parent demande le rendu
        {
            parent_->surface_->pushSurface(surface_.get(), getX(), getY());
        }
        else // le parent ne demande pas de rendu ou le parent n'existe pas
        {

            if (parent_ != nullptr)
            {
                graphics::getLCD()->setClipRect(
                    std::max(parent_->getAbsoluteX(), getAbsoluteX()),
                    std::max(parent_->getAbsoluteY(), getAbsoluteY()),
                    std::min(
                        parent_->getAbsoluteX() + parent_->getWidth(),
                        getAbsoluteX() + getWidth()
                    ) - std::max(parent_->getAbsoluteX(), getAbsoluteX()),
                    std::min(
                        parent_->getAbsoluteY() + parent_->getHeight(),
                        getAbsoluteY() + getHeight()
                    ) - std::max(parent_->getAbsoluteY(), getAbsoluteY())
                );
            }
            else
            {
                graphics::getLCD()
                    ->setClipRect(getAbsoluteX(), getAbsoluteY(), getWidth(), getHeight());
            }

#if defined(ESP_PLATFORM) && defined(USE_DOUBLE_BUFFERING)
            // swap buffers for double buffering

            if (useDoubleBuffering_ && false)
            {
                if (surface_.get() == nullptr)
                    return;

                graphics::getLCD()->waitDMA();
                dmaSurface_.swap(surface_);
                graphics::getLCD()->pushImageDMA(
                    getAbsoluteX(),
                    getAbsoluteY(),
                    dmaSurface_.get()->getWidth(),
                    dmaSurface_.get()->getHeight(),
                    dmaSurface_.get()->m_sprite.getBuffer(),
                    lgfx::color_depth_t::rgb565_2Byte,
                    dmaSurface_.get()->m_sprite.getPalette()
                );
            }
            else
            {
                // push the surface to the screen
                graphics::showSurface(surface_.get(), getAbsoluteX(), getAbsoluteY());
            }

#else
            graphics::showSurface(m_surface.get(), getAbsoluteX(), getAbsoluteY());
#endif

            setChildrenDrawn();
        }
    }
}

bool gui::ElementBase::updateAll()
{
    if (!isEnabled_)
        return false;

    if (!isInside())
    {
        if (surface_ != nullptr)
            free();
        return false;
    }

    if (parent_ == nullptr)
    {
        // StandbyMode::wait();
        if (mainWindow != this)
        {
            mainWindow = this;
            this->isDrawn_ = false;
        }

        graphics::getTouchPos(&touchX, &touchY);
    }

    if (!isDrawn_)
        renderAll();

    bool returnV = false;

    for (auto child : children_)
    {
        if (!child->isEnabled())
            continue;

        if (child->updateAll())
        {
            returnV = true; // if child had an event, ignore local events
            break;
        }
    }

    update();

    if (this->parent_ == nullptr)
        graphics::touchIsRead();

    return returnV;
}

gui::ElementBase* gui::ElementBase::getHighestXScrollableParent()
{
    if (horizontalScrollEnabled_)
        return this;
    else if (parent_ != nullptr)
        return parent_->getHighestXScrollableParent();
    else
        return nullptr;
}

gui::ElementBase* gui::ElementBase::getHighestYScrollableParent()
{
    if (verticalScrollEnabled_)
        return this;
    else if (parent_ != nullptr)
        return parent_->getHighestYScrollableParent();
    else
        return nullptr;
}

bool gui::ElementBase::update()
{
    // algorithme de mise a jour des interactions tactiles

    if (!this->isEnabled_)
        return false;

    widgetUpdate();

    if (!hasEvents_ && widgetPressed != this)
        return false;

    if (widgetPressed != nullptr && widgetPressed != this)
        return false;

    bool isScreenTouched = graphics::isTouched();

    bool isWidgetTouched =
        isScreenTouched && (getAbsoluteX() - hitboxFactor_ < touchX &&
                            touchX < getAbsoluteX() + getWidth() + hitboxFactor_ &&
                            getAbsoluteY() - hitboxFactor_ < touchY &&
                            touchY < getAbsoluteY() + getHeight() + hitboxFactor_);

    bool returnValue = false;

    if (isScreenTouched == false && widgetPressed == nullptr)
        return false;

    // std::cout << "globalPressedState: " << globalPressedState << std::endl;
    // std::cout << "widgetPressed: " << int(widgetPressed != nullptr) <<
    // std::endl;

    if (isScreenTouched)
    {
        if (isWidgetTouched && globalPressedState == NOT_PRESSED)
        {
            globalPressedState = PRESSED;
            widgetPressed = this;

            originTouchX = touchX;
            originTouchY = touchY;

            lastTouchX = touchX;
            lastTouchY = touchY;

            onClick();

            returnValue = true;
        }
        else if (widgetPressed == this)
        {
            bool isScrollingX = abs(lastTouchX - touchX) > SCROLL_STEP;
            bool isScrollingY = abs(lastTouchY - touchY) > SCROLL_STEP;
            bool isScrolling = isScrollingX || isScrollingY;

            if (isScrollingX)
            {
                gui::ElementBase* nearScrollableObject = getHighestXScrollableParent();
                globalPressedState = SCROLL_X;

                if (nearScrollableObject == this)
                {
                    while (lastTouchX + SCROLL_STEP < touchX)
                    {
                        onScrollRight();
                        lastTouchY += SCROLL_STEP;
                    }
                    while (lastTouchX - SCROLL_STEP > touchX)
                    {
                        onScrollLeft();
                        lastTouchY -= SCROLL_STEP;
                    }
                }
                else if (nearScrollableObject == nullptr)
                {
                    globalPressedState = LOCKED;
                }
                else
                {
                    widgetPressed = nearScrollableObject;
                }

                globalPressedState = SCROLL_X;
                lastTouchX = touchX;

                returnValue = true;
            }

            if (isScrollingY)
            {
                gui::ElementBase* nearScrollableObject = getHighestYScrollableParent();
                globalPressedState = SCROLL_Y;

                if (nearScrollableObject == this)
                {
                    onScroll(touchX - lastTouchX, touchY - lastTouchY);

                    while (lastTouchY + SCROLL_STEP < touchY)
                    {
                        onScrollUp();
                        lastTouchY += SCROLL_STEP;
                    }
                    while (lastTouchY - SCROLL_STEP > touchY)
                    {
                        onScrollDown();
                        lastTouchY -= SCROLL_STEP;
                    }
                }
                else if (nearScrollableObject == nullptr)
                {
                    globalPressedState = LOCKED;
                    this->onNotClicked();
                }
                else
                {
                    widgetPressed = getHighestYScrollableParent();
                    this->onNotClicked();
                }

                globalPressedState = SCROLL_Y;
                lastTouchY = touchY;

                returnValue = true;
            }
        }
    }
    else
    {
        onNotClicked();

        if (globalPressedState == PRESSED)
        {
            this->pressedState_ = RELEASED;
            lastEventTouchX = originTouchX;
            lastEventTouchY = originTouchY;
            onReleased();
        }

        globalPressedState = NOT_PRESSED;
        widgetPressed = nullptr;

        returnValue = true;
    }

    wasPressed = isScreenTouched;

    return returnValue;
}

void gui::ElementBase::setX(uint16_t x)
{
    x_ = x;
    globalGraphicalUpdate();
}

void gui::ElementBase::setY(uint16_t y)
{
    y_ = y;
    globalGraphicalUpdate();
}

void gui::ElementBase::setWidth(uint16_t width)
{
    width_ = width;
    globalGraphicalUpdate();
}

void gui::ElementBase::setHeight(uint16_t height)
{
    height_ = height;
    globalGraphicalUpdate();
}

int16_t gui::ElementBase::getAbsoluteX() const
{
    if (parent_ == nullptr)
        return getX();

    return parent_->getAbsoluteX() + getX();
}

int16_t gui::ElementBase::getAbsoluteY() const
{
    if (parent_ == nullptr)
        return getY();

    return parent_->getAbsoluteY() + getY();
}

int16_t gui::ElementBase::getX() const
{
    return (int) x_ /* + (m_parent!=nullptr)?(m_parent->m_horizontalScroll):(0)*/;
}

int16_t gui::ElementBase::getY() const
{
    if (parent_ != nullptr)
        return y_ - parent_->verticalScroll_;
    return y_;
}

uint16_t gui::ElementBase::getWidth() const
{
    return width_;
}

uint16_t gui::ElementBase::getHeight() const
{
    return height_;
}

void gui::ElementBase::setBackgroundColor(const color_t color)
{
    backgroundColor_ = color;
    localGraphicalUpdate();
}

void gui::ElementBase::setBorderColor(const color_t color)
{
    borderColor_ = color;
    localGraphicalUpdate();
}

void gui::ElementBase::setRadius(uint16_t r)
{
    this->borderRadius_ = r;
}

uint16_t gui::ElementBase::getRadius() const
{
    return borderRadius_;
}

void gui::ElementBase::setBorderSize(uint16_t size)
{
    this->borderSize_ = size;
}

uint16_t gui::ElementBase::getBorderSize() const
{
    return borderSize_;
}

color_t gui::ElementBase::getBackgroundColor() const
{
    return backgroundColor_;
}

color_t gui::ElementBase::getBorderColor() const
{
    return borderColor_;
}

bool gui::ElementBase::isTouched()
{
    hasEvents_ = true;
    if (pressedState_ == PressedState::RELEASED)
    {
        pressedState_ = PressedState::NOT_PRESSED;
        return true;
    }
    return false;
}

bool gui::ElementBase::isFocused(bool forced)
{
    if (forced)
    {
        return (
            touchX != -1 && touchY != -1 && getAbsoluteX() - 10 < touchX &&
            touchX < getAbsoluteX() + getWidth() + 10 && // l'objet est touché
            getAbsoluteY() - 10 < touchY && touchY < getAbsoluteY() + getHeight() + 10
        );
    }
    hasEvents_ = true;
    return pressedState_ == PressedState::PRESSED;
}

void gui::ElementBase::enable()
{
    setEnabled(true);
}

void gui::ElementBase::disable()
{
    setEnabled(false);
}

void gui::ElementBase::setEnabled(const bool enabled)
{
    if (isEnabled() == enabled)
    {
        // Do nothing when already in this state.
        return;
    }

    isEnabled_ = enabled;
    globalGraphicalUpdate();
}

bool gui::ElementBase::isEnabled() const
{
    return isEnabled_;
}

gui::ElementBase* gui::ElementBase::getMaster()
{
    // We shoud probably remove this function.
    // Because the "Master" widget (and rename it to "main" please)
    // Is for almost every cases the "Screen" widget.
    // So the user already have a reference to it.

    ElementBase* master = this;

    if (this->parent_ != nullptr)
        master = parent_->getMaster();

    return master;
}

gui::ElementBase* gui::ElementBase::getParent() const
{
    return parent_;
}

void gui::ElementBase::addChild(gui::ElementBase* child)
{
    if (child == nullptr)
        throw libsystem::exceptions::RuntimeError("Child can't be null.");

    children_.push_back(child);
    child->parent_ = this;
}

std::shared_ptr<graphics::Surface> gui::ElementBase::getAndSetSurface()
{
    if (this->surface_ == nullptr)
        this->surface_ = std::make_shared<graphics::Surface>(this->width_, this->height_);
    return this->surface_;
}

void gui::ElementBase::localGraphicalUpdate()
{
    this->isDrawn_ = false;
    this->isRendered_ = false;

    if (parent_ != nullptr)
        setParentNotRendered();
}

void gui::ElementBase::globalGraphicalUpdate()
{
    this->isDrawn_ = false;
    this->isRendered_ = false;

    if (this->parent_ != nullptr)
        setParentNotDrawn();
}

void gui::ElementBase::setParentNotRendered()
{
    if (parent_ != nullptr)
        parent_->setParentNotRendered();

    this->isRendered_ = false;
}

void gui::ElementBase::setParentNotDrawn()
{
    if (parent_ != nullptr)
        parent_->setParentNotDrawn();

    this->isDrawn_ = false;
    this->isRendered_ = false;
}

void gui::ElementBase::setChildrenDrawn()
{
    isDrawn_ = true;

    for (int i = 0; i < children_.size();
         i++) // dire aux enfants qu'il sont actualisés sur l'écran
        if (children_[i] != nullptr)
            children_[i]->setChildrenDrawn();
}

void gui::ElementBase::setChildrenNotDrawn()
{
    isDrawn_ = false;

    for (int i = 0; i < children_.size();
         i++) // dire aux enfants qu'il sont actualisés sur l'écran
        if (children_[i] != nullptr)
            children_[i]->isDrawn_ = false;
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
    *x = lastTouchX;
    *y = lastTouchY;
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
    if (surface_ != nullptr)
        surface_.reset();

    setParentNotRendered();

    for (auto child : children_) child->free();
}

bool gui::ElementBase::isInside()
{
    if (parent_ == nullptr)
        return true;

    if (getX() + getWidth() < 0)
        return false;
    if (getY() + getHeight() < 0)
        return false;
    if (getX() > parent_->getWidth())
        return false;
    if (getY() > parent_->getHeight())
        return false;

    return true;
}

std::shared_ptr<graphics::Surface> gui::ElementBase::getSurface()
{
    return getAndSetSurface();
}

void gui::ElementBase::forceUpdate()
{
    localGraphicalUpdate();
}

gui::ElementBase* gui::ElementBase::getElementAt(int index)
{

    if (index >= 0 && index < children_.size())
        return children_[index];
    return nullptr;
}

#include "elements/Window.hpp"

void gui::ElementBase::freeRamFor(uint32_t size, ElementBase* window)
{
#ifdef ESP_PLATFORM
    size_t free = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);

    if (free < size + 1000000)
    {
        std::cout << "Not enough RAM, free : " << free << " need : " << size
                  << "\n     -> will free other windows" << std::endl;
        for (auto i : gui::elements::Window::windows)
            if (i != window)
                i->free();
    }
#endif
}
