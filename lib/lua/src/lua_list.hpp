#ifndef LUA_LIST_MODULE
#define LUA_LIST_MODULE

#include "lua_widget.hpp"

/**
 * Liste Verticale
 */
class LuaVerticalList : public LuaWidget
{
  public:
    LuaVerticalList(LuaWidget* parent, int x, int y, int width, int height);

    void addChild(LuaWidget* widget);
    void setSpaceLine(int line);
    void setIndex(int i = 0);
    void setFocus(VerticalList::SelectionFocus focus);
    int getSelected();
    void select(int index);
    void setSelectionColor(color_t color);
    void setAutoSelect(bool autoSelect);
    void specificUpdate();

    void onSelect(sol::protected_function func);

  protected:
    VerticalList* widget = nullptr;
    sol::protected_function onSelectFunc;
};

/**
 * Liste Horizontale
 */
class LuaHorizontalList : public LuaWidget
{
  public:
    LuaHorizontalList(LuaWidget* parent, int x, int y, int width, int height);

    void addChild(LuaWidget* widget);
    void setSpaceLine(int line);

    HorizontalList* widget = nullptr;
};

#endif
