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

   std::shared_ptr<graphics::Surface> loadImage(storage::Path path, uint16_t width, uint16_t height, const color_t backgroundColor = 0xFFFF) {
         // ReSharper disable once CppUseStructuredBinding
         for (const auto& image : images) {
             if (image.path.str() == path.str() && image.width == width && image.height == height) {
                 return image.surface;
             }
         }

         const auto i = graphics::SImage(path);

         // libsystem::log("Image: " + std::to_string(i.getType()) + ", " + std::to_string(i.getWidth()) + ", " + std::to_string(i.getHeight()) + ", " + i.getPath().str());

         ImageLoaded img = {
             path,
             i.getWidth(),
             i.getHeight(),
             std::make_shared<graphics::Surface>(width, height)
         };

         // Clear the background if it's a transparent image ?
         // I guess so ?
         if(i.getType() != graphics::ImageType::BMP) {
             img.surface->clear(backgroundColor);
         }

         img.surface->drawImage(i, 0, 0, width, height);

        images.push_back(img);

        return img.surface;
    }

    void updateImageList()
    {
        for (auto img = images.begin(); img != images.end();)
        {
            // TODO: Refactor this logic
            if (img->surface.unique())
            {
                img = images.erase(img);
                std::cout << "[Image] image deleted" << std::endl;
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
        if ( ! m_surface) {
            //std::cout << "[Image] m_surface is null";
            load(color);
        }
        m_surface->setTransparentColor(color);
        m_surface->setTransparency(true);
    }


    void Image::load(color_t background)
    {
        m_surface = gui::ImagesList::loadImage(this->m_path, this->m_width, this->m_height, background);
        localGraphicalUpdate();
    }
}