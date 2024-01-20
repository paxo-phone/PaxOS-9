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

imgdec::IMGData imgdec::decodeBMP(const uint8_t* rawData)
{
    IMGData bmpData = {};

    bmpData.width = make32(rawData[0x12], rawData[0x13], rawData[0x14], rawData[0x15]);
    bmpData.heigth = make32(rawData[0x16], rawData[0x17], rawData[0x18], rawData[0x19]);

    bmpData.size = make32(rawData[0x22], rawData[0x23], rawData[0x24], rawData[0x25]);

    bmpData.image = new uint8_t[bmpData.size];

    const uint32_t imageDataOffset = make32(rawData[0x0A], rawData[0x0B], rawData[0x0C], rawData[0x0D]);

    uint32_t i = 0;
    while (i < bmpData.size)
    {
        bmpData.image[i] = rawData[imageDataOffset + i];
        i++;
    }

    return bmpData;
}

imgdec::IMGData imgdec::decodePNG(const uint8_t* rawData)
{
    throw std::exception(); // Not yet implemented.
}

imgdec::IMGData imgdec::decodeJPG(const uint8_t* rawData)
{
    throw std::exception(); // Not yet implemented.
}
