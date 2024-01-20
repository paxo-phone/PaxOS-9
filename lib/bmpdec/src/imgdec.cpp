//
// Created by Charles on 20/01/2024.
//

#include "imgdec.hpp"

#include <exception>

// Create a uint16 with 2x uint8 (Little endian)
inline uint16_t make16(const uint8_t v1, const uint8_t v2)
{
    return (v2 << 8) + v1;
}

// Create a uint32 with 4x uint8 (Little endian)
inline uint32_t make32(const uint8_t v1, const uint8_t v2, const uint8_t v3, const uint8_t v4)
{
    return (v4 << 24) + (v3 << 16) + (v2 << 8) + v1;
}

imgdec::IMGData imgdec::decodeHeader(const uint8_t* rawData)
{
    IMGData imgData = {};

    if (rawData[0x00] == 'B' && rawData[0x01] == 'M')
    {
        imgData.type = BMP;

        imgData.width = make32(rawData[0x12], rawData[0x13], rawData[0x14], rawData[0x15]);
        imgData.heigth = make32(rawData[0x16], rawData[0x17], rawData[0x18], rawData[0x19]);
    }
    else
    {
        imgData.type = ERROR; // Unknown image
    }

    return imgData;
}
