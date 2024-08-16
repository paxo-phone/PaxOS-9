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

        std::cout <<"to clear :"<< this << " - nb enfants:" <<this->children.size() << std::endl;
        for (auto & obj :this->children) {
            std::cout << obj << std::endl;
        }
        
        std::cout <<"début effacement"<< std::endl;
        for (auto & obj :this->children) {
            //obj->free();
            std::cout << obj << std::endl;
            delete obj;
        }
//        this->widget->m_children.clear();

}



LuaHorizontalList::LuaHorizontalList(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new HorizontalList(x, y, width, height);
    init(widget, parent);
}
