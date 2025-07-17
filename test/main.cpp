//
// Created by Charlito33 on 02/01/2024.
// Copyright (c) 2024 Charlito33. All rights reserved.
//

#include "gsm/contacts_test.cpp"
#include "gsm/conversation_test.cpp"
#include "i18n/test.cpp"
#include "storage/storage_test.cpp"

#include <gtest/gtest.h>

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    if (RUN_ALL_TESTS())
    {
    }

    return 0;
}
