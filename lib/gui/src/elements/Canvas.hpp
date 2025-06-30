#ifndef CANVAS_HPP
#define CANVAS_HPP

#include "../ElementBase.hpp"

#include <filestream.hpp>

namespace gui::elements
{
    /**
     * @brief It is a simple virtual screen where you can draw pixels or shapes as
     *you want. Useful for games or custom widgets
     **/
    class Canvas final : public ElementBase
    {
      public:
        typedef std::pair<int16_t, int16_t> point_t;

        Canvas(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~Canvas() override;

        void render() override;

        void setPixel(int16_t x, int16_t y, color_t color);

        void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, color_t color);
        void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, color_t color);

        void drawCircle(int16_t x, int16_t y, uint16_t radius, color_t color);
        void fillCircle(int16_t x, int16_t y, uint16_t radius, color_t color);

        void drawRoundRect(
            int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, color_t color
        );
        void fillRoundRect(
            int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t radius, color_t color
        );

        void drawPolygon(std::vector<std::pair<int16_t, int16_t>> vertices, color_t color);
        void fillPolygon(std::vector<std::pair<int16_t, int16_t>> vertices, color_t color);

        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, color_t color);

        void drawText(int16_t x, int16_t y, std::string& text, color_t color, float fontSize = 16);
        void drawTextCentered(
            int16_t x, int16_t y, std::string& text, color_t color,
            bool horizontallyCentered = true, bool verticallyCentered = true, float fontSize = 16
        );
        void drawTextCenteredInRect(
            int16_t x, int16_t y, uint16_t w, uint16_t h, std::string& text, color_t color,
            bool horizontallyCentered = true, bool verticallyCentered = true, float fontSize = 16
        );
    };
} // namespace gui::elements

#endif // CANVAS_HPP
