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

namespace graphics
{
    class Surface
    {
        friend void renderSurface(const Surface *surface);

    public:
        lgfx::LGFX_Sprite m_sprite;

        uint8_t m_r;
        uint8_t m_g;
        uint8_t m_b;

        EFont m_font;
        EFontSize m_fontSize;

    public:
        Surface(uint16_t width, uint16_t height);
        ~Surface();

        [[nodiscard]] uint16_t getWidth() const;
        [[nodiscard]] uint16_t getHeight() const;

        uint16_t getPixel(uint16_t x, uint16_t y);
        void setPixel(uint16_t x, uint16_t y, uint16_t value);

        void pushSurface(Surface *surface, int16_t x, int16_t y);

        void clear(uint8_t r, uint8_t g, uint8_t b);
        void clear();

        void setColor(uint8_t r, uint8_t g, uint8_t b);

        void fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h);
        void drawRect(int16_t x, int16_t y, uint16_t w, uint16_t h);

        void fillRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r);
        void drawRoundRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r);

        void drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

        void drawImage(const Image& image, int16_t x, int16_t y);

        void setFont(EFont font);
        void setFontSize(EFontSize fontSize);
        [[deprecated("Please use : setFontSize")]]
        void setTextScale(uint8_t scale);
        void drawText(const std::string& text, int16_t x, int16_t y);
        void drawTextCentered(const std::string& text, int16_t x, int16_t y, uint16_t w = -1);

        void blur(uint8_t radius);
    };
} // graphics

#endif //SURFACE_HPP
