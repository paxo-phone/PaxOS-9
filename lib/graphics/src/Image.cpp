//
// Created by Charles on 20/01/2024.
//

#include "Image.hpp"

#include "imgdec.hpp"

#include <fstream>

// TODO: Replace this with "storage"
uint64_t getFileSize(const char *filename)
{
    auto inputStream = std::ifstream(filename, std::ios::ate | std::ios::binary);

    return inputStream.tellg();
}

// TODO: Replace this with "storage"
uint8_t * getFileData(const char *filename)
{
    const size_t filesize = getFileSize(filename);

    auto *data = new uint8_t[filesize];

    auto inputStream = std::ifstream(filename, std::ios::binary);

    size_t i = 0;
    while (!inputStream.eof())
    {
        data[i++] = inputStream.get();
    }

    return data;
}

namespace graphics {
    Image::Image(const std::string& filename)
    {
        const size_t fileSize = getFileSize(filename.c_str());

        size = fileSize;
        data = getFileData(filename.c_str());;

        const imgdec::IMGData imageData = imgdec::decodeHeader(data);

        switch (imageData.type)
        {
        case imgdec::ERROR:
            throw std::exception();
        case imgdec::BMP:
            type = BMP;
        case imgdec::PNG:
            type = PNG;
        case imgdec::JPG:
            type = JPG;
        }

        width = imageData.width;
        height = imageData.heigth;
    }

    Image::~Image()
    {
        delete data;
    }

    uint32_t Image::getWidth() const
    {
        return width;
    }

    uint32_t Image::getHeight() const
    {
        return height;
    }

    uint32_t Image::getSize() const
    {
        return size;
    }

    uint8_t* Image::getData() const
    {
        return data;
    }

} // graphics