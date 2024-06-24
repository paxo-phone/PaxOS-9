//
// Created by Charles on 17/01/2024.
//

#include "Label.hpp"

#include <cstdio>
#include <graphics.hpp>
#include <Surface.hpp>
#include <iostream>

namespace gui::elements {
    Label::Label(const uint16_t x, const uint16_t y, const uint16_t width, const uint16_t height)
        : ElementBase(),
        m_text(""),
        m_fontSize(18),
        m_textColor(COLOR_DARK),
        m_textVerticalAlignment(UP),
        m_textHorizontalAlignment(LEFT)
    {
        m_x = x;
        m_y = y;
        m_width = width;
        m_height = height;
    }

    Label::~Label() = default;

    void Label::render()
    {
        m_surface->clear(COLOR_WHITE);
        m_surface->fillRoundRectWithBorder(0, 0, m_width, m_height, m_borderRadius, m_borderSize, m_backgroundColor, m_borderColor);

        m_surface->setTextColor((this->m_textColor == 0)?(1):(this->m_textColor));
        m_surface->setColor(this->m_backgroundColor);
        m_surface->setFontSize(this->m_fontSize);

        std::vector<std::string> lines = parse();

        for (size_t i = 0; i < lines.size(); i++)
        {
            int x;
            switch (int(m_textHorizontalAlignment))
            {
                case Alignement::LEFT:
                    x = getRadius()/2 + getBorderSize();
                break;
                case Alignement::CENTER:
                    x = getRadius()/2 + getBorderSize() + getUsableWidth()/2 - m_surface->getTextWidth(lines[i])/2;
                break;
                case Alignement::RIGHT:
                    x = getRadius()/2 + getBorderSize() + getUsableWidth() - m_surface->getTextWidth(lines[i]);
                break;
            };

            int y;
            switch (int(m_textVerticalAlignment))
            {
                case Alignement::UP:
                    y = getRadius()/2 + getBorderSize() + (m_surface->getTextHeight() + LINE_SPACING) * i;
                break;
                case Alignement::CENTER:
                    y = getRadius()/2 + getBorderSize() + getUsableHeight()/2
                        - ((m_surface->getTextHeight() + LINE_SPACING) * lines.size()) / 2 
                        + (m_surface->getTextHeight() + LINE_SPACING) * i;
                break;
                case Alignement::DOWN:
                    y = getRadius()/2 + getBorderSize() + getUsableHeight()
                        - ((m_surface->getTextHeight() + LINE_SPACING) * lines.size())
                        + (m_surface->getTextHeight() + LINE_SPACING) * i;
                break;
            };

            m_surface->drawText(lines[i], x, y);
        }
    }

    void Label::setText(const std::string& text)
    {
        this->m_text = text;
        localGraphicalUpdate();
    }

    std::string Label::getText() const
    {
        return this->m_text;
    }

    void Label::setTextColor(color_t color)
    {
        this->m_textColor = color;
        localGraphicalUpdate();
    }

    std::vector<std::string> Label::parse(void)
    {
        std::vector<std::string> lines;
        std::string currentLine = "";

        

        for (char c : m_text) {
            if (c == '\n') {
                lines.push_back(currentLine);
                currentLine = "";
            } else if (m_surface->getTextWidth(currentLine + c) <= getUsableWidth()) {
                currentLine += c;
            } else if (c == ' ') {
                lines.push_back(currentLine);
                currentLine = "";
            } else {
                if (currentLine.empty()) {
                    currentLine += c;
                } else if (currentLine.back() == ' ') {
                    currentLine += c;
                } else {
                    std::size_t lastSpace = currentLine.find_last_of(' ');
                    if (lastSpace == std::string::npos) {
                        lines.push_back(currentLine);
                        currentLine = "";
                        currentLine += c;
                    } else {
                        std::string firstPart = currentLine.substr(0, lastSpace);
                        lines.push_back(firstPart);
                        currentLine = currentLine.substr(lastSpace + 1);
                        currentLine += c;
                    }
                }
            }
        }

        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }

        return lines;
    }

    uint16_t Label::getUsableWidth(void)
    {
        return getWidth()-getRadius()-2*getBorderSize();
    }

    uint16_t Label::getUsableHeight(void)
    {
        return getHeight()-getRadius()-2*getBorderSize();
    }

    void Label::setHorizontalAlignment(Alignement alignment)
    {
        this->m_textHorizontalAlignment = alignment;
    }

    void Label::setVerticalAlignment(Alignement alignment)
    {
        this->m_textVerticalAlignment = alignment;
    }

    void Label::setFontSize(uint16_t fontSize)
    {
        this->m_fontSize = fontSize;
        localGraphicalUpdate();
    }

    uint16_t Label::getTextWidth()
    {
        if(m_surface == nullptr)
            m_surface = std::make_shared<graphics::Surface>(m_width, m_height);
        m_surface->setFontSize(this->m_fontSize);
        return m_surface->getTextWidth(m_text);
    }

    uint16_t Label::getTextHeight()
    {
        bool allocatedSprite = false;
        if(m_surface == nullptr)
        {
            m_surface = std::make_shared<graphics::Surface>(1, 1);
            allocatedSprite = true;
        }

        m_surface->setFontSize(this->m_fontSize);
        
        std::vector<std::string> lines = parse();
        uint16_t out = getRadius() + getBorderSize()*2 + (m_surface->getTextHeight() + LINE_SPACING) * lines.size();

        if(allocatedSprite)
            m_surface = nullptr;

        return out;
    }
} // gui::elements