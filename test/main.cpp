//
// Created by Charlito33 on 02/01/2024.
// Copyright (c) 2024 Charlito33. All rights reserved.
//

#include "gsm/contacts_test.cpp"
#include "gsm/conversation_test.cpp"
#include "i18n/test.cpp"
#include "imgdec/imgdec_test_bmp.cpp"
#include "imgdec/imgdec_test_jpg.cpp"
#include "imgdec/imgdec_test_png.cpp"
#include "imgdec/imgdec_test_utils.cpp"
#include "storage/storage_test.cpp"

#include <gtest/gtest.h>
#if ESP_PLATFORM
#include "serialcom/command.cpp"
#endif

#if ESP_PLATFORM

void setup() {
    ::testing::InitGoogleTest();

    if (RUN_ALL_TESTS()) {}

    return;
}

void loop() {}

#else

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS()) {}

    return 0;
}

#endif
