//
// Created by Charles on 20/01/2024.
//

#include "imgdec.hpp"

#include <exception>

// Create a uint16 with 2x uint8 (Little endian)
inline uint16_t make16L(const uint8_t v1, const uint8_t v2)
{
    return (v2 << 8) + v1;
}

// Create a uint32 with 4x uint8 (Little endian)
inline uint32_t make32L(const uint8_t v1, const uint8_t v2, const uint8_t v3, const uint8_t v4)
{
    return (v4 << 24) + (v3 << 16) + (v2 << 8) + v1;
}

// Create a uint16 with 2x uint8 (Big endian)
inline uint16_t make16B(const uint8_t v1, const uint8_t v2)
{
    return (v1 << 8) + v2;
}

// Create a uint32 with 4x uint8 (Big endian)
inline uint32_t make32B(const uint8_t v1, const uint8_t v2, const uint8_t v3, const uint8_t v4)
{
    return (v1 << 24) + (v2 << 16) + (v3 << 8) + v4;
}

imgdec::IMGData imgdec::decodeHeader(const uint8_t* rawData)
{
    IMGData imgData = {};

    if (rawData[0x00] == 'B' && rawData[0x01] == 'M')
    {
        imgData.type = BMP;

        imgData.width = make32L(rawData[0x12], rawData[0x13], rawData[0x14], rawData[0x15]);
        imgData.heigth = make32L(rawData[0x16], rawData[0x17], rawData[0x18], rawData[0x19]);
    }
    else if (rawData[0x01] == 'P' && rawData[0x02] == 'N' && rawData[0x03] == 'G')
    {
        imgData.type = PNG;

        imgData.width = make32B(rawData[0x10], rawData[0x11], rawData[0x12], rawData[0x13]);
        imgData.heigth = make32B(rawData[0x14], rawData[0x15], rawData[0x16], rawData[0x17]);
    }
    else if (rawData[0x00] == 0xFF && rawData[0x01] == 0xD8 && rawData[0x02] == 0xFF)
    {
        imgData.type = JPG;

        // JPG is such a weird image format, we need to find the address of the width and height
        int i = 0;
        while (rawData[i] != 0xC0) // Only SOF0 marker, may cause issues
        {
            i++;
        }

        // Skip SOF0 marker
        i++;

        // Skip length
        i += 2;

        // Skip bitsPerSample
        i++;

        imgData.width = make16B(rawData[i], rawData[i + 1]);
        imgData.heigth = make16B(rawData[i + 2], rawData[i + 3]);
    }
    else
    {
        imgData.type = ERROR; // Unknown image
    }

    return imgData;
}
