//
// Created by Charles on 20/01/2024.
//

#include <gtest/gtest.h>
#include <imgdec.hpp>

TEST(IMGDECTestPNG, GetFilesize) {
    const size_t fileSize = getFileSize("resources/images/logo.png");

    EXPECT_NE(fileSize, -1); // Check if no error occurred
    EXPECT_GT(fileSize, 0);  // Check if the file is not empty
}

TEST(IMGDECTestPNG, FileSignature) {
    const uint8_t* fileData = getFileData("resources/images/logo.png");

    EXPECT_EQ(fileData[0x01], 0x50); // P
    EXPECT_EQ(fileData[0x02], 0x4E); // N
    EXPECT_EQ(fileData[0x03], 0x47); // G

    delete fileData;
}

TEST(IMGDECTestPNG, ImageType) {
    const uint8_t* fileData = getFileData("resources/images/logo.png");

    const imgdec::IMGData imgData = imgdec::decodeHeader(fileData);

    EXPECT_EQ(imgData.type, imgdec::PNG);

    delete fileData;
}

TEST(IMGDECTestPNG, ImageWidthHeight) {
    const uint8_t* fileData = getFileData("resources/images/logo.png");

    const imgdec::IMGData imgData = imgdec::decodeHeader(fileData);

    EXPECT_EQ(imgData.width, 302);
    EXPECT_EQ(imgData.heigth, 302);

    delete fileData;
}
