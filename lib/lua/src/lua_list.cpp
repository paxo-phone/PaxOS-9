#include "lua_list.hpp"

LuaVerticalList::LuaVerticalList(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new VerticalList(x, y, width, height);
    init(widget, parent);
}

void LuaVerticalList::addChild(LuaWidget* widget)
{ 
    this->children.push_back(widget); 
    widget->parent = this; 
    this->widget->add(widget->widget); 
}

void LuaVerticalList::clear() { 

        while (!this->children.empty()) {
            delete this->children[0];
        }       
        this->widget->renderAll();
}



LuaHorizontalList::LuaHorizontalList(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new HorizontalList(x, y, width, height);
    init(widget, parent);
}

void LuaHorizontalList::clear() { 

    while (!this->children.empty()) {
        delete this->children[0];
    }       
}
