//
// Created by Charles on 20/01/2024.
//

#include <imgdec.hpp>
#include <gtest/gtest.h>

TEST(IMGDECTestBMP, GetFilesize)
{
    const size_t fileSize = getFileSize("resources/images/logo.bmp");

    EXPECT_NE(fileSize, -1); // Check if no error occurred
    EXPECT_GT(fileSize, 0); // Check if the file is not empty
}

TEST(IMGDECTestBMP, FileSignature)
{
    const uint8_t *fileData = getFileData("resources/images/logo.bmp");

    EXPECT_EQ(fileData[0x00], 0x42); // B
    EXPECT_EQ(fileData[0x01], 0x4D); // M

    delete fileData;
}

TEST(IMGDECTestBMP, ImageType)
{
    const uint8_t *fileData = getFileData("resources/images/logo.bmp");

    const imgdec::IMGData imgData = imgdec::decodeHeader(fileData);

    EXPECT_EQ(imgData.type, imgdec::BMP);

    delete fileData;
}


TEST(IMGDECTestBMP, ImageWidthHeight)
{
    const uint8_t *fileData = getFileData("resources/images/logo.bmp");

    const imgdec::IMGData imgData = imgdec::decodeHeader(fileData);

    EXPECT_EQ(imgData.width, 302);
    EXPECT_EQ(imgData.heigth, 302);

    delete fileData;
}
