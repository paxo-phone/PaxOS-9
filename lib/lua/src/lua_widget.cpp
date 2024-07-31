#include "lua_widget.hpp"
#include "lua_gui.hpp"

LuaWidget* LuaWidget::rootOfDelete = nullptr;

void LuaWidget::init(gui::ElementBase* obj, LuaWidget* parent)
{
    widget = obj;
    parent->addChild(this);
}

LuaWidget::~LuaWidget()
{
    if(this->widget == nullptr)
        return;
    
    if(rootOfDelete == nullptr)
        rootOfDelete = this;

    for (LuaWidget* child : children)
    {
        delete child;
    }

    if(rootOfDelete == this && this->parent != nullptr)
    {
        LuaWidget* widget = this;
        gui::ElementBase* reWidget = widget->widget;

        LuaWidget* parent = this->parent;
        gui::ElementBase* reParent = parent->widget;
        
        parent->children.erase(std::remove_if(parent->children.begin(), parent->children.end(), // remove from abstract parent this
                [&](LuaWidget* obj) {
                    return obj == widget;
                }
            ), parent->children.end());

        reParent->m_children.erase(std::remove_if(reParent->m_children.begin(), reParent->m_children.end(),   // remove from real parent the real widget
                [&](gui::ElementBase* obj) {
                    return obj == reWidget;
                }
            ), reParent->m_children.end());
    }

    gui->widgets.erase(std::remove_if(gui->widgets.begin(), gui->widgets.end(), // remove from abstract parent this
                [&](LuaWidget* obj) {
                    return obj == this;
                }
            ), gui->widgets.end());
    
    if(rootOfDelete == this)
    {
        delete this->widget;
        rootOfDelete = nullptr;
    }
        this->widget = nullptr;
}

void LuaWidget::update()
{
    if(this->widget->getParent() == nullptr)
    {
        this->widget->updateAll();
    }

    specificUpdate();

    if(onClickFunc && this->widget->isTouched())
    {
        //std::cout << "[Lua] User Interacted" << std::endl;
        onClickFunc();
    }

    /*if(onScrollUpFunc && touch.isSlidingVertically() < -30)
    {
        onScrollUpFunc();
        touch.resetScrollVertical();
    }

    if(onScrollDownFunc && touch.isSlidingVertically() > 30)
    {
        onScrollDownFunc();
        touch.resetScrollVertical();
    }

    if(onScrollRightFunc && touch.isSlidingHorizontally() > 30)
    {
        onScrollRightFunc();
        touch.resetScrollHorizontal();
    }

    if(onScrollLeftFunc && touch.isSlidingHorizontally() < -30)
    {
        onScrollLeftFunc();
        touch.resetScrollHorizontal();
    }*/

    for (int i = 0; i < this->children.size(); i++)
    {
        children[i]->update();
    }
}