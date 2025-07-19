//
// Created by Charles on 20/01/2024.
//

#include "Image.hpp"

#include "imgdec.hpp"

#include <filestream.hpp>
#include <fstream>
#include <libsystem.hpp>
#include <memory>
#include <path.hpp>

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
    auto data = std::shared_ptr<uint8_t[]>(new uint8_t[2000]); // just for headers

    storage::FileStream stream(path.str(), storage::Mode::READ);

    if (!stream.isopen())
        return data;

    size_t i = 0;
    while (i < 2000) data.get()[i++] = stream.readchar();

    stream.close();

    return data;
}

namespace graphics
{
    SImage::SImage(storage::Path& path)
    {
        this->m_path = path;

        if (!path.exists() || !path.isfile())
        {
            std::cerr << "Path does not exist : " << path.str() << std::endl;
            // throw libsystem::exceptions::InvalidArgument("Path does not exist : " + path.str() +
            // ".");    // trop radical
            m_width = 0;
            m_height = 0;
            return;
        }

        m_data = getFileData(path);

        const imgdec::IMGData imageData = imgdec::decodeHeader(m_data.get());

        switch (imageData.type)
        {
        case imgdec::ERROR:
            throw libsystem::exceptions::InvalidArgument("Invalid image data.");
            // m_width = 0;
            // m_height = 0;
            // return;
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

    uint16_t SImage::getWidth() const
    {
        return m_width;
    }

    uint16_t SImage::getHeight() const
    {
        return m_height;
    }

    uint32_t SImage::getSize() const
    {
        return m_size;
    }

    uint8_t* SImage::getData() const
    {
        return m_data.get();
    }

    storage::Path SImage::getPath() const
    {
        return this->m_path;
    }

} // namespace graphics
