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
                //std::cout << "image already loaded" << std::endl;
                return image.surface;
            }
        }

        //std::cout << "image not loaded" << std::endl;

        graphics::SImage i = graphics::SImage(path);

        ImageLoaded img = { 
            path, 
            i.getWidth(), 
            i.getHeight(), 
            std::make_shared<graphics::Surface>(i.getWidth(), i.getHeight()) 
        };
        
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
                //std::cout << "[Image] image deleted" << std::endl;
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

    void Image::setTransparentColor(color_t color){
        m_surface->setTransparency(true);
        m_surface->setTransparentColor(color);
    }

    void Image::load(color_t background)
    {
        m_surface = gui::ImagesList::loadImage(this->m_path, this->m_width, this->m_height, background);
        localGraphicalUpdate();
    }
}