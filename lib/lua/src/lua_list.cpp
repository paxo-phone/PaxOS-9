#include "lua_list.hpp"

/**
 * LuaVerticalList
 */
LuaVerticalList::LuaVerticalList(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new VerticalList(x, y, width, height);
    init(widget, parent);
    widget->setBackgroundColor(
        widget->m_parent == nullptr ? COLOR_WHITE : widget->m_parent->getBackgroundColor()
    );
}

void LuaVerticalList::addChild(LuaWidget* widget)
{
    this->children.push_back(widget);
    widget->parent = this;
    this->widget->add(widget->widget);
}

void LuaVerticalList::setSpaceLine(int line)
{
    this->widget->setSpaceLine(line);
}

void LuaVerticalList::setIndex(int i)
{
    this->widget->setIndex(i);
}

void LuaVerticalList::setFocus(VerticalList::SelectionFocus focus)
{
    this->widget->setSelectionFocus(focus);
}

int LuaVerticalList::getSelected()
{
    return this->widget->getFocusedElement();
}

void LuaVerticalList::onSelect(sol::protected_function func)
{
    onSelectFunc = func;
}

void LuaVerticalList::select(int index)
{
    this->widget->select(index);
    if (onSelectFunc)
        onSelectFunc();
}

void LuaVerticalList::setSelectionColor(color_t color)
{
    this->widget->setSelectionColor(color);
}

void LuaVerticalList::setAutoSelect(bool autoSelect)
{
    this->widget->setAutoSelect(autoSelect);
}

void LuaVerticalList::specificUpdate()
{

    //    LuaWidget:update();

    if (onSelectFunc && this->widget->getIsSelected())
    {
        onSelectFunc();
        this->widget->setIsSelected(false);
    }
}

/**
 * LuaHorizontalList
 */

LuaHorizontalList::LuaHorizontalList(LuaWidget* parent, int x, int y, int width, int height)
{
    widget = new HorizontalList(x, y, width, height);
    init(widget, parent);
}

void LuaHorizontalList::addChild(LuaWidget* widget)
{
    this->children.push_back(widget);
    widget->parent = this;
    this->widget->add(widget->widget);
}

void LuaHorizontalList::setSpaceLine(int line)
{
    this->widget->setSpaceLine(line);
}
