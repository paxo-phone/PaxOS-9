//
// Created by Charlito33 on 02/01/2024.
// Copyright (c) 2024 Charlito33. All rights reserved.
//

#include <gtest/gtest.h>

#include "i18n/test.cpp"

#include "imgdec/imgdec_test_utils.cpp"
#include "imgdec/imgdec_test_bmp.cpp"
#include "imgdec/imgdec_test_png.cpp"
#include "imgdec/imgdec_test_jpg.cpp"

#include "storage/storage_test.cpp"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS())
    {}

    return 0;
}