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

    void Image::load(color_t background)
    {
        try
        {
            graphics::SImage i = graphics::SImage(this->m_path);
            
            m_surface = std::make_shared<graphics::Surface>(i.getWidth(), i.getHeight());
            this->m_width = i.getWidth();
            this->m_height = i.getHeight();
            m_surface->clear(background);

            m_surface->drawImage(i, 0, 0);

            localGraphicalUpdate();
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            std::cout << "Failed to load image -> crashes" << std::endl;
            std::cout << "Path: " << this->m_path.str() << std::endl;
            std::cout << "Size: " << this->m_width << ", " << this->m_height << std::endl;
            if (m_surface == nullptr)
            {
                std::cout << "Surface is nullptr" << std::endl;
            
            #ifdef ESP_PLATFORM
                // Print the size of the psram if the ESP_PLATFORM is defined
                size_t psram_size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
                std::cout << "Size of psram: " << psram_size << " bytes" << std::endl;
            #endif
            }
        }
        
    }
}