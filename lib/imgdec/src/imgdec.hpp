//
// Created by Charles on 20/01/2024.
//

#ifndef IMGDEC_HPP
#define IMGDEC_HPP

#include <cstdint>

namespace imgdec
{
    enum IMGType
    {
        ERROR,
        BMP,
        PNG,
        JPG
    };

    typedef struct IMGData IMGData;
    struct IMGData
    {
        IMGType type = ERROR;

        uint32_t width = -1;
        uint32_t heigth = -1;
    };

    IMGData decodeHeader(const uint8_t *rawData);
}

#endif //IMGDEC_HPP
