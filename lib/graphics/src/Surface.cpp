//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "Surface.hpp"

#include <graphics.hpp>
#include <iostream>

#include "color.hpp"
#include "fonts/Arial-12.h"
#include "fonts/Arial-16.h"
#include "fonts/Arial-24.h"
#include "fonts/Arial-8.h"

#include "Encoder/decodeutf8.hpp"

namespace graphics
{
    Surface::Surface(const uint16_t width, const uint16_t height) : m_color(0xFFFF),
                                                                    m_transparent(false),
                                                                    m_transparent_color(0xFFFF),
                                                                    m_font(ARIAL),
                                                                    m_fontSize(PT_12)
    {
        m_sprite.setColorDepth(16);
        m_sprite.setPsram(true);

        m_sprite.createSprite(width, height);
    }

    Surface::~Surface()
    {
        m_sprite.deleteSprite();
    }

    uint16_t Surface::getWidth() const
    {
        return m_sprite.width();
    }

    uint16_t Surface::getHeight() const
    {
        return m_sprite.height();
    }

    uint16_t Surface::getPixel(const uint16_t x, const uint16_t y)
    {
        return m_sprite.readPixel(x, y);
    }

    void Surface::setPixel(const uint16_t x, const uint16_t y, const color_t value)
    {
        m_sprite.writePixel(x, y, value);
    }

    void Surface::pushSurface(Surface *surface, const int16_t x, const int16_t y)
    {
        if (surface->m_transparent)
        {
            surface->m_sprite.pushSprite(
                &m_sprite,
                x,
                y,
                surface->m_transparent_color);
        }
        else
        {
            surface->m_sprite.pushSprite(
                &m_sprite,
                x,
                y);
        }
    }

    void Surface::pushSurfaceWithScale(Surface *surface, const int16_t x, const int16_t y, const float scale)
    {
        // LovyanGFX is very weird...
        // When pushing with "pushRotateZoomWithAA",
        // the x and y are the coordinates of the CENTER of the sprite
        // and not the top-left corner as in "pushSprite"
        // The calcs are working, PLEASE DON'T TOUCH THEM

        if (surface->m_transparent)
        {
            surface->m_sprite.pushRotateZoomWithAA(
                &m_sprite,
                static_cast<float>(x) + static_cast<float>(surface->getWidth()) * scale * 0.5f,
                static_cast<float>(y) + static_cast<float>(surface->getHeight()) * scale * 0.5f,
                0,
                scale,
                scale,
                surface->m_transparent_color);
        }
        else
        {
            surface->m_sprite.pushRotateZoomWithAA(
                &m_sprite,
                static_cast<float>(x) + static_cast<float>(surface->getWidth()) * scale * 0.5f,
                static_cast<float>(y) + static_cast<float>(surface->getHeight()) * scale * 0.5f,
                0,
                scale,
                scale);
        }
    }

    void Surface::clear(const color_t color)
    {
        m_sprite.clear(color);
    }

    void Surface::clear()
    {
        m_sprite.clear();
    }

    void Surface::setColor(const color_t color)
    {
        m_color = color;
    }

    void Surface::setTransparency(const bool enabled)
    {
        m_transparent = enabled;
    }

    void Surface::setTransparentColor(const color_t color)
    {
        m_transparent_color = color;
    }

    void Surface::fillRect(const int16_t x, const int16_t y, const uint16_t w, const uint16_t h, const color_t color)
    {
        m_sprite.fillRect(x, y, w, h, color);
    }

    void Surface::drawRect(const int16_t x, const int16_t y, const uint16_t w, const uint16_t h, const color_t color)
    {
        m_sprite.drawRect(x, y, w, h, color);
    }

    void Surface::fillRoundRect(const int16_t x, const int16_t y, const uint16_t w, const uint16_t h, const uint16_t r, const color_t color)
    {
        m_sprite.fillSmoothRoundRect(x, y, w, h, r, color);
    }

    void Surface::fillRoundRectWithBorder(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, int16_t bs, uint16_t Backcolor, uint16_t Bordercolor)
    {
        m_sprite.fillSmoothRoundRect(x, y, w, h, r, Bordercolor);
        m_sprite.fillSmoothRoundRect(x + bs, y + bs, w - 2 * bs, h - 2 * bs, r - bs, Backcolor);
    }

    void Surface::drawRoundRect(const int16_t x, const int16_t y, const uint16_t w, const uint16_t h, const uint16_t r, const color_t color)
    {
        m_sprite.drawRoundRect(x, y, w, h, r, color);
    }

    void Surface::drawLine(const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const color_t color)
    {
        m_sprite.drawLine(
            x1,
            y1,
            x2,
            y2,
            color);
    }

    void Surface::drawImage(const SImage &image, const int16_t x, const int16_t y)
    {
        switch (image.getType())
        {
        case BMP:
            m_sprite.drawBmp(image.getData(), image.getSize(), x, y, static_cast<int16_t>(image.getWidth()), static_cast<int16_t>(image.getHeight()));
            break;
        case PNG:
            m_sprite.drawPng(image.getData(), image.getSize(), x, y, static_cast<int16_t>(image.getWidth()), static_cast<int16_t>(image.getHeight()));
            break;
        case JPG:
            m_sprite.drawJpg(image.getData(), image.getSize(), x, y, static_cast<int16_t>(image.getWidth()), static_cast<int16_t>(image.getHeight()));
            break;
        }
    }

    void Surface::setFont(const EFont font)
    {
        m_font = font;
    }

    void Surface::setFontSize(const float fontSize)
    {
        m_fontSize = fontSize;
    }

    void Surface::setTextColor(color_t color)
    {
        this->m_text_color = color;
    }

    const lgfx::GFXfont *getFont(const EFont font, const EFontSize fontSize)
    {
        if (font == ARIAL)
        {
            if (fontSize == PT_8)
                return &Arial8ptFR;
            if (fontSize == PT_12)
                return &Arial12ptFR;
            if (fontSize == PT_16)
                return &Arial16ptFR;
            if (fontSize == PT_24)
                return &Arial24ptFR;
        }

        return nullptr;
    }

    uint16_t Surface::getTextWidth(std::string text)
    {
        EFontSize fontSize;

        if (m_fontSize < 18)
            fontSize = PT_8;
        else if (m_fontSize < 27)
            fontSize = PT_12;
        else if (m_fontSize < 35)
            fontSize = PT_16;
        else
            fontSize = PT_24;

        const float scale = m_fontSize / static_cast<float>(m_sprite.fontHeight(getFont(m_font, fontSize)));

        this->m_sprite.setFont(getFont(m_font, fontSize));

        return (float) scale * this->m_sprite.textWidth(decodeString(text).c_str());
    }

    uint16_t Surface::getTextHeight() const
    {
        EFontSize fontSize;

        if (m_fontSize < 18)
            fontSize = PT_8;
        else if (m_fontSize < 27)
            fontSize = PT_12;
        else if (m_fontSize < 35)
            fontSize = PT_16;
        else
            fontSize = PT_24;

        const float scale = m_fontSize / static_cast<float>(m_sprite.fontHeight(getFont(m_font, fontSize)));

        return (float) scale * m_sprite.fontHeight(getFont(m_font, fontSize));
    }

    void Surface::drawText(std::string &otext, const int16_t x, const int16_t y)
    {
        std::string text = decodeString(otext);
        // Get the correct font size
        EFontSize fontSize;

        if (m_fontSize < 18)
            fontSize = PT_8;
        else if (m_fontSize < 27)
            fontSize = PT_12;
        else if (m_fontSize < 35)
            fontSize = PT_16;
        else
            fontSize = PT_24;

        // Get the correct scale
        const float scale = m_fontSize / static_cast<float>(m_sprite.fontHeight(getFont(m_font, fontSize)));

        // Get buffer size
        const uint16_t bufferHeight = m_sprite.fontHeight(getFont(m_font, fontSize));
        const uint16_t bufferWidth = bufferHeight;

        // Create buffer
        auto *buffer = new Surface(bufferWidth, bufferHeight);

        // Init the buffer
        buffer->m_sprite.setFont(getFont(m_font, fontSize));
        buffer->m_sprite.setTextColor(m_text_color);

        // Set the background as transparent
        buffer->setTransparency(true);
        buffer->setTransparentColor(m_color);

        // Init variables
        float ox = 0; // X offset

        // Draw text char by char
        for (size_t i = 0; i < text.length(); i++)
        {
            // Reset buffer
            buffer->clear(m_color);

            // Get char
            const char c = text[i];
            const char *str = std::string(1, c).c_str(); // Bad trick I think

            // Draw the char
            buffer->m_sprite.drawString(str, 0, 0);

            // Push the buffer
            pushSurfaceWithScale(buffer, static_cast<int16_t>(static_cast<float>(x) + ox), y, scale);

            // Update the offset
            const uint16_t charWidth = m_sprite.textWidth(str, getFont(m_font, fontSize));
            ox += static_cast<float>(charWidth) * scale;
        }

        delete buffer;
    }

    void Surface::drawTextCentered(std::string &text, const int16_t x, const int16_t y, const uint16_t w)
    {
        const uint16_t textWidth = m_sprite.textWidth(text.c_str());

        if (w == -1)
        {
            drawText(text, static_cast<int16_t>(x + getScreenWidth() * 0.5 - textWidth * 0.5), y);
        }
        else
        {
            drawText(text, static_cast<int16_t>(x + w * 0.5 - textWidth * 0.5), y);
        }
    }

    void Surface::blur(uint8_t radius)
    {
        // Copy
        auto copy = lgfx::LGFX_Sprite();
        copy.createSprite(getWidth(), getHeight());

        // Apply blur effect
        for (uint16_t x = radius; x < getWidth() - radius; x++)
        {
            for (uint16_t y = radius; y < getHeight() - radius; y++)
            {
                uint64_t sumR = 0;
                uint64_t sumG = 0;
                uint64_t sumB = 0;

                const uint8_t blurSize = pow((radius * 2 + 1), 2);

                // Read nearby pixel values
                for (uint8_t i = 0; i <= blurSize; i++)
                {
                    const uint8_t dx = i % (radius * 2 + 1);
                    const uint8_t dy = i / (radius * 2 + 1);

                    const uint16_t color = m_sprite.readPixel(x + dx, y + dy);

                    uint8_t r, g, b;
                    unpackRGB565(color, &r, &g, &b);

                    sumR += r;
                    sumG += g;
                    sumB += b;
                }

                sumR /= blurSize;
                sumG /= blurSize;
                sumB /= blurSize;

                uint16_t color = packRGB565(sumR, sumG, sumB);

                copy.writePixel(x, y, color);
            }
        }

        // Update the Surface
        copy.pushSprite(&m_sprite, 0, 0);
    }
} // graphics