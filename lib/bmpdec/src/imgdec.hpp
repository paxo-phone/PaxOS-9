//
// Created by Charles on 20/01/2024.
//

#ifndef IMGDEC_HPP
#define IMGDEC_HPP

#include <cstdint>

namespace imgdec
{
    typedef struct IMGData IMGData;
    struct IMGData
    {
        uint32_t width = -1;
        uint32_t heigth = -1;

        uint32_t size = -1;

        uint8_t *image = nullptr;
    };

    IMGData decodeBMP(const uint8_t *rawData);
    IMGData decodePNG(const uint8_t *rawData);
    IMGData decodeJPG(const uint8_t *rawData);
}

#endif //IMGDEC_HPP
