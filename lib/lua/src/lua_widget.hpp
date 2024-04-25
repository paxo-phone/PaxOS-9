#ifndef LUA_MODULE_WIDGET
#define LUA_MODULE_WIDGET

#include <gui.hpp>
#include "SOL2/sol.hpp"
#include "SOL2/sol_helper.hpp"

class LuaWidget
{
    public:
    void init(gui::ElementBase* obj, LuaWidget* parent);
    void init(gui::ElementBase* obj) {widget = obj;}
    ~LuaWidget();

    void setX(int x){this->widget->setX(x);}
    void setY(int y){this->widget->setY(y);}
    void setWidth(int width){this->widget->setWidth(width);}
    void setHeight(int height){this->widget->setHeight(height);}
    int getX(){return this->widget->getX();}
    int getY(){return this->widget->getY();}
    int getWidth(){return this->widget->getWidth();}
    int getHeight(){return this->widget->getHeight();}
    void setBackgroundColor(color_t color){this->widget->setBackgroundColor(color);}
    void enable(){this->widget->enable();}
    void disable(){this->widget->disable();}
    bool isTouched(){return this->widget->isFocused();}
    void onClick(sol::function func){onClickFunc = func;}
    void onScrollUp(sol::function func){onScrollUpFunc = func;}
    void onScrollDown(sol::function func){onScrollDownFunc = func;}
    void onScrollRight(sol::function func){onScrollRightFunc = func;}
    void onScrollLeft(sol::function func){onScrollLeftFunc = func;}

    void update();

    gui::ElementBase* widget = nullptr;
    LuaWidget* parent = nullptr;
    std::vector<LuaWidget*> children;
    
    private:
    void addChild(LuaWidget* child){this->children.push_back(child); child->parent = this;}

    sol::function onClickFunc;
    sol::function onScrollUpFunc;
    sol::function onScrollDownFunc;
    sol::function onScrollRightFunc;
    sol::function onScrollLeftFunc;
};

#endif