//
// Created by Charlito33 on 26/12/2023.
// Copyright (c) 2023 Charlito33. All rights reserved.
//

#include "Surface.hpp"

#include <filestream.hpp>
#include <path.hpp>
// #include <JPEGENC.h>
#include <algorithm>
#include <iostream>

#ifdef ESP_PLATFORM
#include <Arduino.h>
#include <SD.h>
#endif

#ifndef JPEG_PIXEL_RGB565
#define JPEG_PIXEL_RGB565 4
#endif

#ifndef JPEG_SUBSAMPLE_444
#define JPEG_SUBSAMPLE_444 0
#endif

#include "Encoder/decodeutf8.hpp"
#include "color.hpp"
#include "fonts/Arial-12.h"
#include "fonts/Arial-16.h"
#include "fonts/Arial-24.h"
#include "fonts/Arial-8.h"
#include "fonts/ArialBold-12.h"
#include "fonts/ArialBold-16.h"
#include "fonts/ArialBold-24.h"
#include "fonts/ArialBold-8.h"

/*void print_memory_info() {
    // Get total and free heap size
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t total_heap = 270000;

    // Get largest free block
    uint32_t largest_free_block =
heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);

    // Get PSRAM info (if available)
    uint32_t psram_free = 0;
    uint32_t psram_size = 0;
    uint32_t psram_largest_free_block = 0;

    if (esp_spiram_is_initialized()) {
        psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
        psram_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        psram_largest_free_block =
heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    }

    // Print memory info
    printf("Memory Info:\n");
    printf("Internal RAM - Total: %u, Free: %u, Largest Free Block: %u\n",
           total_heap, free_heap, largest_free_block);

    if (esp_spiram_is_initialized()) {
        printf("PSRAM - Total: %u, Free: %u, Largest Free Block: %u\n",
               psram_size, psram_free, psram_largest_free_block);
    } else {
        printf("PSRAM not initialized or not available\n");
    }

    // Get stack high water mark
    UBaseType_t stack_hwm = uxTaskGetStackHighWaterMark(NULL);
    printf("Stack High Water Mark: %u\n", stack_hwm);
}*/

namespace graphics
{
    Surface::Surface(const uint16_t width, const uint16_t height, const uint8_t color_depth) :
        m_color(0xFFFF), m_transparent(false), m_transparent_color(0xFFFF), m_font(ARIAL),
        m_fontSize(PT_12), m_color_depth(color_depth)
    {
        m_sprite.setColorDepth(m_color_depth);
        m_sprite.setPsram(true);

        m_sprite.createSprite(width, height);
        if (m_sprite.getBuffer() == nullptr)
        {
            std::cerr << "[Error] Unable to allocate a new surface: " << width * height * 2
                      << " bytes" << std::endl;
        }
        else
        {
            const double size = width * height * 2;
            const double k = 1024;
            const double m = k * k;

            // if(size < k)
            // {
            //     std::cout << "[Debug] New surface: " << size << " bytes" <<
            //     std::endl;
            // }
            // else if(size < m)
            // {
            //     std::cout << "[Debug] New surface: " << size / k << " Ko" <<
            //     std::endl;
            // }
            // else
            // {
            //     std::cout << "[Debug] New surface: " << size / m << " Mo" <<
            //     std::endl;
            // }
        }
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

    void Surface::pushSurface(Surface* surface, const int16_t x, const int16_t y)
    {
        if (surface == nullptr)
            return;

        if (surface->m_transparent)
            surface->m_sprite.pushSprite(&m_sprite, x, y, surface->m_transparent_color);
        else
            surface->m_sprite.pushSprite(&m_sprite, x, y);
    }

    void Surface::pushSurfaceWithScale(
        Surface* surface, const int16_t x, const int16_t y, const float scale
    )
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
                surface->m_transparent_color
            );
        }
        else
        {
            surface->m_sprite.pushRotateZoomWithAA(
                &m_sprite,
                static_cast<float>(x) + static_cast<float>(surface->getWidth()) * scale * 0.5f,
                static_cast<float>(y) + static_cast<float>(surface->getHeight()) * scale * 0.5f,
                0,
                scale,
                scale
            );
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

    void Surface::drawRect(
        const int16_t x, const int16_t y, const uint16_t w, const uint16_t h, const color_t color
    )
    {
        m_sprite.drawRect(x, y, w, h, color);
    }

    void Surface::fillRect(
        const int16_t x, const int16_t y, const uint16_t w, const uint16_t h, const color_t color
    )
    {
        m_sprite.fillRect(x, y, w, h, color);
    }

    void Surface::drawCircle(int16_t x, int16_t y, uint16_t r, color_t color)
    {
        this->m_sprite.drawCircle(x, y, r, color);
    }

    void Surface::fillCircle(int16_t x, int16_t y, uint16_t r, color_t color)
    {
        this->m_sprite.fillCircle(x, y, r, color);
    }

    void Surface::fillRoundRect(
        const int16_t x, const int16_t y, const uint16_t w, const uint16_t h, const uint16_t r,
        const color_t color
    )
    {
        m_sprite.fillSmoothRoundRect(x, y, w, h, r, color);
    }

    void Surface::fillRoundRectWithBorder(
        int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, int16_t bs, uint16_t Backcolor,
        uint16_t Bordercolor
    )
    {
        m_sprite.fillSmoothRoundRect(x, y, w, h, r, Bordercolor);
        m_sprite.fillSmoothRoundRect(x + bs, y + bs, w - 2 * bs, h - 2 * bs, r, Backcolor);
    }

    void Surface::drawRoundRect(
        const int16_t x, const int16_t y, const uint16_t w, const uint16_t h, const uint16_t r,
        const color_t color
    )
    {
        m_sprite.drawRoundRect(x, y, w, h, r, color);
    }

    void Surface::drawLine(
        const int16_t x1, const int16_t y1, const int16_t x2, const int16_t y2, const color_t color
    )
    {
        m_sprite.drawLine(x1, y1, x2, y2, color);
    }

    void Surface::drawImage(
        const SImage& image, const int16_t x, const int16_t y, const uint16_t w, const uint16_t h
    )
    {
        // printf("IMG--3-drawImage-1\n");
        float scaleX = static_cast<float>(w) / static_cast<float>(image.getWidth());
        float scaleY = static_cast<float>(h) / static_cast<float>(image.getHeight());

        if (m_sprite.getBuffer() == nullptr)
        {
            std::cerr << "[Error] Unable to write on an invalid sprite: " << w * h * 2 << " bytes"
                      << std::endl;
            return;
        }

#ifdef ESP_PLATFORM
        if (image.getType() == PNG)
        {
            storage::FileStream stream(image.getPath().str(), storage::Mode::READ);
            size_t size = stream.size();

            if (!stream.isopen() || size == 0)
                return;

            std::cout << "Reading PNG...: " << size << std::endl;
            char* buffer = new char[size];
            stream.read(buffer, size);
            stream.close();
            std::cout << "End Reading PNG...: " << size << std::endl;

            m_sprite.drawPng((uint8_t*) buffer, size, x, y, 0, 0, 0, 0, scaleX, scaleY);
            m_sprite.releasePngMemory();
            // here is the place to code the decompression

            delete[] buffer;
            std::cout << "End Reading PNG...: " << size << std::endl;

            return;
        }
        switch (image.getType()) // image size with right format
        {
        case BMP:
            m_sprite.drawBmpFile(image.getPath().str().c_str(), x, y, 0, 0, 0, 0, scaleX, scaleY);
            break;
        case PNG:
            // printf("IMG--3-drawImage-2\n");
            m_sprite.drawPngFile(image.getPath().str().c_str(), x, y, 0, 0, 0, 0, scaleX, scaleY);
            // printf("IMG--3-drawImage-3\n");
            break;
        case JPG:
            m_sprite.drawJpgFile(image.getPath().str().c_str(), x, y, 0, 0, 0, 0, scaleX, scaleY);
            break;
        };

#else
        lgfx::FileWrapper file;

        switch (image.getType())
        {
        case BMP:
            m_sprite.drawBmpFile(
                &file,
                image.getPath().str().c_str(),
                x,
                y,
                0,
                0,
                0,
                0,
                scaleX,
                scaleY
            );
            break;
        case PNG:
            m_sprite.drawPngFile(
                &file,
                image.getPath().str().c_str(),
                x,
                y,
                0,
                0,
                0,
                0,
                scaleX,
                scaleY
            );
            break;
        case JPG:
            m_sprite.drawJpgFile(
                &file,
                image.getPath().str().c_str(),
                x,
                y,
                0,
                0,
                0,
                0,
                scaleX,
                scaleY
            );
            break;
        };
#endif
    }

    bool Surface::saveAsJpg(const storage::Path filename)
    {
        /*JPEG jpg;
        int quality = 90;

        LGFX_Sprite sprite = &m_sprite;

        if (sprite.getBuffer() == nullptr || !filename.str().size()) {
            Serial.println("Invalid sprite or filename");
            return false;
        }

        int width = sprite.width();
        int height = sprite.height();

        // Open the file for writing
        std::ofstream outFile(filename.str(), std::ios::binary);
        if (!outFile.is_open()) {
            Serial.println("Failed to open file for writing");
            return false;
        }

        // JPEG encoder object
        JPEGENCODE jpe;

        // Start the JPEG encoding process
        int rc = jpg.encodeBegin(&jpe, width, height, JPEG_PIXEL_RGB565,
        JPEG_SUBSAMPLE_444, quality); if (rc != 0) { Serial.println("Failed to start
        JPEG encoding"); outFile.close(); return false;
        }

        // Calculate MCU dimensions
        int mcu_w = (width + jpe.cx - 1) / jpe.cx;
        int mcu_h = (height + jpe.cy - 1) / jpe.cy;

        // Buffer for one MCU
        uint8_t ucMCU[64 * 3]; // 8x8 pixels, 3 channels (RGB)

        // Process each MCU
        for (int y = 0; y < mcu_h; y++) {
            for (int x = 0; x < mcu_w; x++) {
                // Extract MCU data from sprite
                for (int j = 0; j < 8; j++) {
                    for (int i = 0; i < 8; i++) {
                        int px = x * 8 + i;
                        int py = y * 8 + j;
                        if (px < width && py < height) {
                            uint16_t pixel = sprite.readPixel(px, py);
                            // Convert RGB565 to RGB888
                            uint8_t r = ((pixel >> 11) & 0x1F) << 3;
                            uint8_t g = ((pixel >> 5) & 0x3F) << 2;
                            uint8_t b = (pixel & 0x1F) << 3;
                            ucMCU[(j * 8 + i) * 3] = r;
                            ucMCU[(j * 8 + i) * 3 + 1] = g;
                            ucMCU[(j * 8 + i) * 3 + 2] = b;
                        } else {
                            // Fill with black if outside sprite bounds
                            ucMCU[(j * 8 + i) * 3] = 0;
                            ucMCU[(j * 8 + i) * 3 + 1] = 0;
                            ucMCU[(j * 8 + i) * 3 + 2] = 0;
                        }
                    }
                }

                // Add MCU to JPEG
                rc = jpg.addMCU(&jpe, ucMCU, 8);
                if (rc != 0) {
                    Serial.println("Failed to add MCU");
                    outFile.close();
                    return false;
                }
            }
        }

        // Finish encoding and get the compressed data
        int jpegSize = jpg.close();

        // Write JPEG data to file
        outFile.write(reinterpret_cast<const char*>(jpe.pOutput), jpegSize);
        outFile.close();

        Serial.print("JPEG file created: ");
        Serial.println(filename);
        Serial.print("File size: ");
        Serial.print(jpegSize);
        Serial.println(" bytes");*/

        return true;
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

    const lgfx::GFXfont* getFont(const EFont font, const EFontSize fontSize)
    {
        if (font == ARIAL)
        {
            if (fontSize == PT_8)
                return &ArialBold8ptFR;
            if (fontSize == PT_12)
                return &ArialBold12ptFR;
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

        const float scale =
            m_fontSize / static_cast<float>(m_sprite.fontHeight(getFont(m_font, fontSize)));

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

        const float scale =
            m_fontSize / static_cast<float>(m_sprite.fontHeight(getFont(m_font, fontSize)));

        return (float) scale * m_sprite.fontHeight(getFont(m_font, fontSize));
    }

    void Surface::drawText(std::string& otext, int16_t x, int16_t y, std::optional<color_t> color)
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
        const float scale =
            m_fontSize / static_cast<float>(m_sprite.fontHeight(getFont(m_font, fontSize)));

        // Get buffer size
        const uint16_t bufferHeight = m_sprite.fontHeight(getFont(m_font, fontSize));
        const uint16_t bufferWidth = bufferHeight;

        // Create buffer
        // FIXME : Probably can optimize it with a LovyanGFX direct sprite
        auto* buffer = new Surface(bufferWidth, bufferHeight);

        // Init the buffer
        buffer->m_sprite.setFont(getFont(m_font, fontSize));
        if (color.has_value())
            buffer->m_sprite.setTextColor(color.value());
        else
            buffer->m_sprite.setTextColor(m_text_color);

        // Set the background as transparent
        // FIXME : Using "m_color" as the transparency color can cause issues
        // Example : if your color is the same as your text color
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
            const char* str = std::string(1, c).c_str(); // Bad trick I think

            // Draw the char
            buffer->m_sprite.drawString(str, 0, 0);

            // Push the buffer
            pushSurfaceWithScale(
                buffer,
                static_cast<int16_t>(static_cast<float>(x) + ox),
                y,
                scale
            );

            // Update the offset
            const uint16_t charWidth = m_sprite.textWidth(str, getFont(m_font, fontSize));
            ox += static_cast<float>(charWidth) * scale;
        }

        delete buffer;
    }

    void Surface::drawTextCentered(
        std::string& text, const int16_t x, const int16_t y, const uint16_t w, const uint16_t h,
        const bool horizontallyCentered, const bool verticallyCentered,
        const std::optional<color_t> color
    )
    {
        const uint16_t textWidth = m_sprite.textWidth(text.c_str());
        int16_t textPositionX;
        if (horizontallyCentered)
            if (w == (uint16_t) -1)
                textPositionX = x + (double) this->getWidth() * 0.5 - (double) textWidth * 0.5;
            else
                textPositionX = x + (double) w * 0.5 - (double) textWidth * 0.5;
        else
            textPositionX = x;

        const uint16_t textHeight =
            this->getTextHeight(); // maybe it should take in account the bounding box height
        int16_t textPositionY;
        if (verticallyCentered)
            if (h == (uint16_t) -1)
                textPositionY = y + (double) this->getHeight() * 0.5 - (double) textHeight * 0.5;
            else
                textPositionY = y + (double) h * 0.5 - (double) textHeight * 0.5;
        else
            textPositionY = y;

        drawText(text, textPositionX, textPositionY, color);
    }

    Surface Surface::clone() const
    {
        auto output = Surface(getWidth(), getHeight());

        output.m_sprite.setBuffer(m_sprite.getBuffer(), m_sprite.width(), m_sprite.height());

        return output;
    }

    void* Surface::getBuffer() const
    {
        return m_sprite.getBuffer();
    }

    void Surface::setBuffer(void* buffer, int32_t w, int32_t h)
    {
        if (w == -1)
            w = getWidth();
        if (h == -1)
            h = getHeight();

        m_sprite.setBuffer(buffer, w, h, m_sprite.getColorDepth());
    }

    void Surface::applyFilter(const Filter filter, const int32_t intensity)
    {
        switch (filter)
        {
        case BLUR:
            blur(intensity);
            break;
        case LIGHTEN:
            lighten(intensity);
            break;
        case DARKEN:
            darken(intensity);
            break;
        default:;
        }
    }

    void Surface::blur(const int32_t radius)
    {
        // Copy
        auto copy = lgfx::LGFX_Sprite();
        copy.setColorDepth(m_color_depth);
        copy.createSprite(getWidth(), getHeight());

        // Apply blur effect
        for (int32_t x = radius; x < m_sprite.width(); x++)
        {
            for (int32_t y = radius; y < m_sprite.height(); y++)
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

    void Surface::fastBlur(int32_t radius)
    {
        // TODO
    }

    void Surface::lighten(const int32_t intensity)
    {
        for (int32_t x = 0; x < getWidth(); x++)
        {
            for (int32_t y = 0; y < getHeight(); y++)
            {
                uint8_t r, g, b;
                unpackRGB565(m_sprite.readPixel(x, y), &r, &g, &b);

                r = std::clamp(r + intensity, 0, 255);
                g = std::clamp(g + intensity, 0, 255);
                b = std::clamp(b + intensity, 0, 255);

                m_sprite.writePixel(x, y, packRGB565(r, g, b));
            }
        }
    }

    void Surface::darken(const int32_t intensity)
    {
        lighten(-intensity);
    }
} // namespace graphics
