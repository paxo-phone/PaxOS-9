//
// Created by Charles on 20/01/2024.
//

#include "imgdec.hpp"

#include <filestream.hpp>
#include <iostream>

#define TJE_IMPLEMENTATION
#include "toojpeg.h"

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
    else
    {
        imgData.type = JPG;

        // Look for the SOF0 marker (FF C0) which indicates the start of frame
        int i = 0;
        while (i < 2000) // A reasonable limit for searching within the file
        {
            if (rawData[i] == 0xFF && rawData[i + 1] == 0xC0)
            {
                // SOF0 marker found
                imgData.heigth = make16B(rawData[i + 5], rawData[i + 6]);
                imgData.width = make16B(rawData[i + 7], rawData[i + 8]);
                break;
            }
            i++;
        }

        // If we reach the end of the loop without finding the SOF0 marker, it's
        // an error
        if (i >= 2000)
            std::cerr << "Invalid JPEG file: SOF0 marker not found." << std::endl;
    }
    /*else
    {
        imgData.type = ERROR; // Unknown image
    }*/

    return imgData;
}

std::ofstream myFile;

// write a single byte compressed by tooJpeg
void myOutput(unsigned char byte)
{
    myFile << byte;
}

void imgdec::encodeJpg(
    const uint8_t* rawData, uint32_t width, uint32_t height, storage::Path filename
)
{
    std::string path = filename.str();

    myFile.open(path, std::ios::binary);

    // RGB: one byte each for red, green, blue
    const auto bytesPerPixel = 3;

    // start JPEG compression
    // note: myOutput is the function defined in line 18, it saves the output in
    // example.jpg optional parameters:
    const bool isRGB = true;       // true = RGB image, else false = grayscale
    const auto quality = 90;       // compression quality: 0 = worst, 100 = best, 80
                                   // to 90 are most often used
    const bool downsample = false; // false = save as YCbCr444 JPEG (better
                                   // quality), true = YCbCr420 (smaller file)
    const char* comment = "mms";   // arbitrary JPEG comment

    std::cout << "ready to write: " << path << std::endl;

    auto ok =
        TooJpeg::writeJpeg(myOutput, rawData, width, height, isRGB, quality, downsample, comment);

    myFile.close();
}
