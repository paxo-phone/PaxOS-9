#include "Image.hpp"

#include <imgdec.hpp>
#include <filestream.hpp>
#include <path.hpp>

#include <iostream>

namespace gui::ImagesList
{
    struct ImageLoaded
    {
        storage::Path path;
        uint16_t width;
        uint16_t height;
        std::shared_ptr<graphics::Surface> surface;
        color_t backgroundColor;
    };

    std::vector<ImageLoaded> images;

    std::shared_ptr<graphics::Surface> loadImage(storage::Path path, uint16_t width, uint16_t height, color_t backgroundColor = 0xFFFF)
    {
        for (const auto& image : images)
        {
            if (image.path.str() == path.str() && image.width == width && image.height == height)
            {
                std::cout << "image already loaded" << std::endl;
                return image.surface;
            }
        }

        std::cout << "image not loaded" << std::endl;

        graphics::SImage i = graphics::SImage(path);

        ImageLoaded img = { path, i.getWidth(), i.getHeight(), std::make_shared<graphics::Surface>(i.getWidth(), i.getHeight()) };
        
        uint16_t m_width = i.getWidth();
        uint16_t m_height = i.getHeight();

        if(i.getType() != graphics::ImageType::BMP)
            img.surface->clear(backgroundColor);
        img.surface->drawImage(i, 0, 0);

        images.push_back(img);

        return img.surface;
    }

    void updateImageList()
    {
        for (auto img = images.begin(); img != images.end();)
        {
            if (img->surface.use_count() == 1)
            {
                img = images.erase(img);
                std::cout << "image deleted ----------------" << std::endl;
            }
            else
            {
                ++img;
            }
        }
    }
}

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

    Image::~Image()
    {
        ImagesList::updateImageList();
    }

    void Image::render()
    {
        if(m_isRendered == false)
            load(m_backgroundColor);
    }

    void Image::load(color_t background)
    {
        /*try
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
        }*/
        
        m_surface = gui::ImagesList::loadImage(this->m_path, this->m_width, this->m_height, background);
        localGraphicalUpdate();
    }
}