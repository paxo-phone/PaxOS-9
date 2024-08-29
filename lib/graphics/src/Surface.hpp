//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#ifndef SURFACE_HPP
#define SURFACE_HPP

#include <cstdint>

#include "LovyanGFX.hpp"

#include "Image.hpp"
#include "fonts.hpp"
#include "color.hpp"
#include <optional>

namespace graphics
{
    class SImage;
    
    class Surface
    {
        friend void showSurface(const Surface *surface);

    public:
        lgfx::LGFX_Sprite m_sprite;

        color_t m_color;
        bool m_transparent;
        color_t m_transparent_color;

        EFont m_font;
        float m_fontSize;
        color_t m_text_color;

    public:
        Surface(uint16_t width, uint16_t height);
        ~Surface();

        [[nodiscard]] uint16_t getWidth() const;
        [[nodiscard]] uint16_t getHeight() const;

        uint16_t getPixel(uint16_t x, uint16_t y);
        void setPixel(uint16_t x, uint16_t y, color_t value);

        void pushSurface(Surface *surface, int16_t x, int16_t y);
        void pushSurfaceWithScale(Surface *surface, int16_t x, int16_t y, float scale);

        void clear(color_t color);
        void clear();

        void setColor(const color_t color);

        void setTransparency(bool enabled);
        void setTransparentColor(color_t color);

        void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, color_t color);
        void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, color_t color);

        void drawCircle(int16_t x, int16_t y, uint16_t r, color_t color);
        void fillCircle(int16_t x, int16_t y, uint16_t r, color_t color);

        void fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, color_t color);
        void fillRoundRectWithBorder(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, int16_t bs, uint16_t Backcolor, uint16_t Bordercolor);
        void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, color_t color);

        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, color_t color);

        void drawImage(const SImage &image, int16_t x, int16_t y, uint16_t w = 0, uint16_t h = 0);

        void setFont(EFont font);
        void setFontSize(float fontSize);
        void setTextColor(const color_t textColor);
        uint16_t getTextWidth(std::string text);
        uint16_t getTextHeight() const;
        void drawText(std::string &otext, int16_t x, int16_t y, std::optional<color_t> color = std::nullopt);

        // w and h are the width and height of the bounding box where the text will be centered
        void drawTextCentered(std::string &text, const int16_t x, const int16_t y, const uint16_t w = -1, const uint16_t h = -1, const bool horizontallyCentered = true, const bool verticallyCentered = true, const std::optional<color_t> color = std::nullopt);

        void blur(uint8_t radius);
    };
} // graphics

#endif // SURFACE_HPP