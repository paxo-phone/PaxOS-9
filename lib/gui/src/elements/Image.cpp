#include "Image.hpp"

#include <imgdec.hpp>
#include <filestream.hpp>
#include <path.hpp>

#include <iostream>

namespace gui::elements
{
    Image::Image(storage::Path path, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t backgroundColor)
    {
        this->m_path = path;
        this->m_x = x;
        this->m_y = y;
        this->m_width = width;
        this->m_height = height;

        this->m_backgroundColor = backgroundColor;
    }

    Image::~Image() = default;

    void Image::render()
    {
    }

    void Image::load()
    {
        graphics::SImage i = graphics::SImage(this->m_path);
        
        m_surface = std::make_shared<graphics::Surface>(i.getWidth(), i.getHeight());
        this->m_width = i.getWidth();
        this->m_height = i.getHeight();
        m_surface->clear(m_backgroundColor);
        m_surface->drawImage(i, 0, 0);/**/

        localGraphicalUpdate();
    }
}