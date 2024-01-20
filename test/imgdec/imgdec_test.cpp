//
// Created by Charles on 20/01/2024.
//

#include <imgdec.hpp>
#include <gtest/gtest.h>

#include <fstream>

uint64_t getFileSize(const char *filename)
{
    auto inputStream = std::ifstream(filename, std::ios::ate | std::ios::binary);

    return inputStream.tellg();
}

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

TEST(IMGDECTest, GetFilesize)
{
    const size_t fileSize = getFileSize("resources/images/logo.bmp");

    EXPECT_NE(fileSize, -1); // Check if no error occurred
    EXPECT_GT(fileSize, 0); // Check if the file is not empty
}

TEST(IMGDECTest, FileSignature)
{
    const uint8_t *fileData = getFileData("resources/images/logo.bmp");

    EXPECT_EQ(fileData[0], 0x42);
    EXPECT_EQ(fileData[1], 0x4D);

    delete fileData;
}

TEST(IMGDECTest, FileSize)
{
    const uint8_t *fileData = getFileData("resources/images/logo.bmp");

    EXPECT_EQ(fileData[0], 0x42);
    EXPECT_EQ(fileData[1], 0x4D);

    delete fileData;
}

TEST(IMGDECTest, ImageWidthHeight)
{
    const uint8_t *fileData = getFileData("resources/images/logo.bmp");

    const imgdec::IMGData imgData = imgdec::decodeHeader(fileData);

    EXPECT_EQ(imgData.width, 302);
    EXPECT_EQ(imgData.heigth, 302);

    delete fileData;
}
