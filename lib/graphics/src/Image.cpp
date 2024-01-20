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

    printf("Filesize : %llu\n", filesize);

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
    Image::Image(const ImageType type) :
        width(-1),
        height(-1),
        size(-1),
        data(nullptr)
    {
        this->type = type;
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

    uint8_t * Image::getData8() const
    {
        return data;
    }

    uint16_t * Image::getData16() const
    {
        throw std::exception();
    }

    uint32_t * Image::getData32() const
    {
        throw std::exception();
    }

    uint8_t* Image::getRawData() const
    {
        return rawData;
    }


    void Image::loadBMP(const std::string& filename)
    {
        rawData = getFileData(filename.c_str());;

        const imgdec::IMGData imageData = imgdec::decodeBMP(rawData);

        width = imageData.width;
        height = imageData.heigth;

        size = imageData.size;
        data = imageData.image;

        printf("HELLO I'M HERE BRO !");
    }

} // graphics