//
// Created by Charles on 20/01/2024.
//

#include "Image.hpp"

#include "imgdec.hpp"

#include <fstream>
#include <memory>

// TODO: Replace this with "storage"
// TODO : Use "Path"
uint64_t getFileSize(const char *filename)
{
    auto inputStream = std::ifstream(filename, std::ios::ate | std::ios::binary);

    return inputStream.tellg();
}

// TODO: Replace this with "storage"
// TODO : Use "Path"
std::shared_ptr<uint8_t[]> getFileData(const char *filename)
{
    const size_t filesize = getFileSize(filename);

    auto data = std::shared_ptr<uint8_t[]>(new uint8_t[filesize]);

    auto inputStream = std::ifstream(filename, std::ios::binary);

    size_t i = 0;
    while (i < filesize)
    {
        data.get()[i++] = inputStream.get();
    }

    return data;
}

namespace graphics {
    // TODO : Use "Path"
    Image::Image(const std::string& filename)
    {
        const size_t fileSize = getFileSize(filename.c_str());

        m_size = fileSize;
        m_data = getFileData(filename.c_str());;

        const imgdec::IMGData imageData = imgdec::decodeHeader(m_data.get());

        switch (imageData.type)
        {
        case imgdec::ERROR:
            // throw std::exception();
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

    Image::~Image() = default;

    ImageType Image::getType() const
    {
        return m_type;
    }

    uint32_t Image::getWidth() const
    {
        return m_width;
    }

    uint32_t Image::getHeight() const
    {
        return m_height;
    }

    uint32_t Image::getSize() const
    {
        return m_size;
    }

    uint8_t * Image::getData() const
    {
        return m_data.get();
    }

} // graphics