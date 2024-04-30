#ifndef LUA_LIST_MODULE
#define LUA_LIST_MODULE

#include "lua_widget.hpp"

class LuaVerticalList : public LuaWidget
{
    public:
    LuaVerticalList(LuaWidget* parent, int x, int y, int width, int height);
    
    void addChild(LuaWidget* widget){ this->children.push_back(widget); widget->parent = this; this->widget->add(widget->widget); }
    void setSpaceLine(int line){  this->widget->setSpaceLine(line); }

    VerticalList* widget = nullptr;
};

class LuaHorizontalList : public LuaWidget
{
    public:
    LuaHorizontalList(LuaWidget* parent, int x, int y, int width, int height);
    
    void addChild(LuaWidget* widget){ this->children.push_back(widget); widget->parent = this; this->widget->add(widget->widget); }
    void setSpaceLine(int line){  this->widget->setSpaceLine(line); }

    HorizontalList* widget = nullptr;
};

#endif