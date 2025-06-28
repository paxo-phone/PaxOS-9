//
// Created by Charles on 20/01/2024.
//

#include <gtest/gtest.h>
#include <imgdec.hpp>

TEST(IMGDECTestJPG, GetFilesize) {
    const size_t fileSize = getFileSize("resources/images/logo.jpg");

    EXPECT_NE(fileSize, -1); // Check if no error occurred
    EXPECT_GT(fileSize, 0);  // Check if the file is not empty
}

TEST(IMGDECTestJPG, FileSignature) {
    const uint8_t* fileData = getFileData("resources/images/logo.jpg");

    EXPECT_EQ(fileData[0x00], 0xFF);
    EXPECT_EQ(fileData[0x01], 0xD8);
    EXPECT_EQ(fileData[0x02], 0xFF);

    delete fileData;
}

TEST(IMGDECTestJPG, ImageType) {
    const uint8_t* fileData = getFileData("resources/images/logo.jpg");

    const imgdec::IMGData imgData = imgdec::decodeHeader(fileData);

    EXPECT_EQ(imgData.type, imgdec::JPG);

    delete fileData;
}

TEST(IMGDECTestJPG, ImageWidthHeight) {
    const uint8_t* fileData = getFileData("resources/images/logo.jpg");

    const imgdec::IMGData imgData = imgdec::decodeHeader(fileData);

    EXPECT_EQ(imgData.width, 302);
    EXPECT_EQ(imgData.heigth, 302);

    delete fileData;
}
