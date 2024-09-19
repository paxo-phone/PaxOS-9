#ifndef LUA_CANVAS_MODULE
#define LUA_CANVAS_MODULE

#include "lua_widget.hpp"

namespace paxolua {
    class LuaEnvironment;
}

class LuaFile;

class LuaCanvas : public LuaWidget
{
    public:
    LuaCanvas(LuaWidget* parent, int x, int y, int width, int height, LuaFile* lua);
    LuaCanvas(LuaWidget* parent, int x, int y, int width, int height, paxolua::LuaEnvironment* env);

    void setPixel(int16_t x, int16_t y, color_t color) { widget->setPixel(x, y, color); }

    void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, color_t color) { widget->drawRect(x, y, w, h, color); }
    void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, color_t color) { widget->fillRect(x, y, w, h, color); }

    void drawCircle(int16_t x, int16_t y, uint16_t radius, color_t color) { widget->drawCircle(x, y, radius, color); }
    void fillCircle(int16_t x, int16_t y,  uint16_t radius, color_t color) { widget->fillCircle(x, y, radius, color); }

    void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, color_t color) { widget->drawRoundRect(x, y, w, h, radius, color); }
    void fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, color_t color) { widget->fillRoundRect(x, y, w, h, radius, color); }
    
    void drawPolygon(sol::table vertices, color_t color) { 
        std::vector<std::pair<int16_t, int16_t>> castedVertices;
        for (std::vector<int16_t> vertex : convert_sequence<std::vector<int16_t>>(vertices)) 
        {
            castedVertices.push_back(std::make_pair(vertex[0], vertex[1]));
        }
        widget->drawPolygon(castedVertices, color); 
    }
    void fillPolygon(sol::table vertices, color_t color) { 
        std::vector<std::pair<int16_t, int16_t>> castedVertices;
        for (std::vector<int16_t> vertex : convert_sequence<std::vector<int16_t>>(vertices)) 
        {
            castedVertices.push_back(std::make_pair(vertex[0], vertex[1]));
        }
        widget->fillPolygon(castedVertices, color); 
    }
    
    void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, color_t color) { widget->drawLine(x1, y1, x2, y2, color); }

    void drawText(int16_t x, int16_t y, std::string text, color_t color) { widget->drawText(x, y, text, color); }

    // w and h are the width and height of the text bounding box
    void drawTextCentered(int16_t x, int16_t y, std::string text, color_t color, bool horizontallyCentered, bool verticallyCentered) { widget->drawTextCentered(x, y, text, color, horizontallyCentered, verticallyCentered); }
    void drawTextCenteredInRect(int16_t x, int16_t y, uint16_t w, uint16_t h, std::string text, color_t color, bool horizontallyCentered, bool verticallyCentered) { widget->drawTextCenteredInRect(x, y, w, h, text, color, horizontallyCentered, verticallyCentered); }

    void onTouch(sol::protected_function function) { this->onTouchFunc = function; }

    sol::table getTouch();
    void specificUpdate();
    
    LuaFile* lua = nullptr;
    gui::elements::Canvas* widget = nullptr;

    sol::protected_function onTouchFunc;

private:
    paxolua::LuaEnvironment *m_env;
};

#endif // LUA_CANVAS_MODULE