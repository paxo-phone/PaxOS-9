#ifndef LUA_LIST_MODULE
#define LUA_LIST_MODULE

#include "lua_widget.hpp"

class LuaVerticalList : public LuaWidget
{
    public:
    LuaVerticalList(LuaWidget* parent, int x, int y, int width, int height);
    
    void add(LuaWidget* widget){  this->widget->add(widget->widget); }
    void setSpaceLine(int line){  this->widget->setSpaceLine(line); }

    VerticalList* widget = nullptr;
};

class LuaHorizontalList : public LuaWidget
{
    public:
    LuaHorizontalList(LuaWidget* parent, int x, int y, int width, int height);
    
    void add(LuaWidget* widget){  this->widget->add(widget->widget); }
    void setSpaceLine(int line){  this->widget->setSpaceLine(line); }

    HorizontalList* widget = nullptr;
};

#endif