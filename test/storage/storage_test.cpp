//
// Created by Charles on 28/01/2024.
//

#include <path.hpp>
#include <filestream.hpp>

#include <gtest/gtest.h>

TEST(StorageTest, PathTest)
{
    storage::init();

    EXPECT_FALSE(storage::Path("/dir1").exists());
    EXPECT_TRUE(storage::Path("/dir1").newdir());
    EXPECT_TRUE(storage::Path("/dir1").exists());
    EXPECT_TRUE(storage::Path("/dir1").isdir());
    EXPECT_FALSE(storage::Path("/dir1").isfile());

    EXPECT_FALSE(storage::Path("/dir1/test.txt").exists());
    EXPECT_TRUE(storage::Path("/dir1/test.txt").newfile());
    EXPECT_TRUE(storage::Path("/dir1/test.txt").isfile());
    EXPECT_FALSE(storage::Path("/dir1/test.txt").isdir());

    EXPECT_TRUE(storage::Path("/dir1/test.txt").rename(storage::Path("/dir1/test2.txt")));
    EXPECT_TRUE(storage::Path("/dir1/test2.txt").exists());
    EXPECT_TRUE(storage::Path("/dir1/test2.txt").remove());
    EXPECT_FALSE(storage::Path("/dir1/test2.txt").exists());

    EXPECT_TRUE(storage::Path("/dir1").remove());
    EXPECT_FALSE(storage::Path("/dir1").exists());
}

TEST(StorageTest, StreamTest)
{
    storage::init();

    const auto file = storage::Path("test.txt");

    EXPECT_FALSE(file.exists());
    EXPECT_TRUE(file.newfile());
    EXPECT_TRUE(file.exists());

    auto fileStream = storage::FileStream();

    fileStream.open(file.str(), storage::Mode::READ);
    EXPECT_EQ(fileStream.read(), "");
    fileStream.close();

    fileStream.open(file.str(), storage::Mode::WRITE);
    fileStream.write("Hello World !");
    fileStream.close();

    fileStream.open(file.str(), storage::Mode::READ);
    EXPECT_EQ(fileStream.read(), "Hello World !");
    fileStream.close();

    EXPECT_TRUE(file.remove());
}