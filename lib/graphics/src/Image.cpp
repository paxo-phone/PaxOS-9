//
// Created by Charles on 20/01/2024.
//

#include "Image.hpp"

namespace graphics {
    Image::Image(const ImageType type)
    {
        this->type = type;
    }

    Image::~Image() = default;

    uint8_t Image::getData8() const
    {

    }

    uint16_t Image::getData16() const
    {

    }

    uint32_t Image::getData32() const
    {

    }
} // graphics