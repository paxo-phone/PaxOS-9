//
// Created by Charles on 20/01/2024.
//

#include "Image.hpp"

#include "imgdec.hpp"

#include <fstream>
#include <path.hpp>
#include <filestream.hpp>
#include <memory>

// TODO: Replace this with "storage"
// TODO : Use "Path"
uint64_t getFileSize(storage::Path& path)
{
   return 0;
}
#include <iostream>

// TODO: Replace this with "storage"
// TODO : Use "Path"
std::shared_ptr<uint8_t[]> getFileData(storage::Path& path)
{
    auto data = std::shared_ptr<uint8_t[]>(new uint8_t[30]);    // just for headers

    storage::FileStream stream(path.str(), storage::Mode::READ);

    if(!stream.isopen())
    {
        std::cout << "Error: " << path.str() << std::endl;
        return data;
    }

    size_t i = 0;
    while (i < 30)
    {
        data.get()[i++] = stream.readchar();
    }

    return data;
}

namespace graphics {
    SImage::SImage(storage::Path& path)
    {
        this->m_path = path;

        if(!path.isfile())
            return;

        m_data = getFileData(path);

        const imgdec::IMGData imageData = imgdec::decodeHeader(m_data.get());

        switch (imageData.type)
        {
        case imgdec::ERROR:
            throw std::exception();
        case imgdec::BMP:
            m_type = BMP;
            break;
        case imgdec::PNG:
            m_type = PNG;
            break;
        case imgdec::JPG:
            m_type = JPG;
            break;
        }

        m_width = imageData.width;
        m_height = imageData.heigth;
    }

    SImage::~SImage() = default;

    ImageType SImage::getType() const
    {
        return m_type;
    }

    uint32_t SImage::getWidth() const
    {
        return m_width;
    }

    uint32_t SImage::getHeight() const
    {
        return m_height;
    }

    uint32_t SImage::getSize() const
    {
        return m_size;
    }

    uint8_t * SImage::getData() const
    {
        return m_data.get();
    }

    storage::Path SImage::getPath() const
    {
        return this->m_path;
    }

} // graphics