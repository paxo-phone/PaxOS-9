#include "lua_widget.hpp"

void LuaWidget::init(gui::ElementBase* obj, LuaWidget* parent)
{
    widget = obj;
    parent->addChild(this);
}

LuaWidget::~LuaWidget()
{
    for (LuaWidget* child : children)
    {
        delete child;
    }

    if (this->widget->getParent() == nullptr)
    {
        delete this->widget;
        this->widget = nullptr;
    }
}

void LuaWidget::update()
{
    if(this->widget->getParent() == nullptr)
        this->widget->updateAll();

    specificUpdate();

    if(onClickFunc && this->widget->isTouched())
    {
        std::cout << "onClickFunc" << std::endl;
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