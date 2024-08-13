#ifndef LUA_MODULE_WIDGET
#define LUA_MODULE_WIDGET

#include <gui.hpp>
#include "SOL2/sol.hpp"
#include "SOL2/sol_helper.hpp"

class LuaGui;

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
    void setBorderColor(color_t color){this->widget->setBorderColor(color);}
    void setBorderSize(uint16_t size){this->widget->setBorderSize(size);}
    void setRadius(uint16_t r){this->widget->setRadius(r);}    
    void enable(){this->widget->enable();}
    void disable(){this->widget->disable();}
    bool isEnabled(){return this->widget->getIsEnabled();}
    bool isTouched(){return this->widget->isFocused();}
    void onClick(sol::protected_function func){onClickFunc = func;}
    void onScrollUp(sol::protected_function func){onScrollUpFunc = func;}
    void onScrollDown(sol::protected_function func){onScrollDownFunc = func;}
    void onScrollRight(sol::protected_function func){onScrollRightFunc = func;}
    void onScrollLeft(sol::protected_function func){onScrollLeftFunc = func;}
    LuaWidget* getChildAtIndex(int index){return children[index];}

    void update();
    virtual void specificUpdate(){};

    gui::ElementBase* widget = nullptr;
    LuaWidget* parent = nullptr;
    std::vector<LuaWidget*> children;

    LuaGui* gui = nullptr;
    
    private:
    virtual void addChild(LuaWidget* child)
    {
        this->children.push_back(child); child->parent = this;
        this->widget->addChild(child->widget);
    }

    static LuaWidget* rootOfDelete;

    sol::protected_function onClickFunc;
    sol::protected_function onScrollUpFunc;
    sol::protected_function onScrollDownFunc;
    sol::protected_function onScrollRightFunc;
    sol::protected_function onScrollLeftFunc;
};

#endif