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
} // graphics