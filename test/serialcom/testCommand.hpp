#pragma once

#include <Command.hpp>
#include <CommandsManager.hpp>
#include <algorithm>
#include <bitset>
#include <gtest/gtest.h>

using namespace serialcom;

void executeCommand(std::string commandText, bool shellMode)
{
    char info_input[INPUT_MAX_SIZE] = {'\0'};
    strcpy(info_input, commandText.c_str());
    Command command = Command(info_input);
    CommandsManager::defaultInstance->shellMode = shellMode;
    SerialManager::sharedInstance->startCommandLog();
    CommandsManager::defaultInstance->processCommand(command);
    SerialManager::sharedInstance->finishCommandLog(CommandsManager::defaultInstance->shellMode);
}

void testCommand(
    std::string commandText, bool shellMode, Command::CommandType expectedCommandType,
    std::string expectedOutput
)
{
    char info_input[INPUT_MAX_SIZE] = {'\0'};
    strcpy(info_input, commandText.c_str());
    Command command = Command(info_input);
    ASSERT_EQ(command.type, expectedCommandType);

    std::stringstream buffer;
    // Redirect std::cout to buffer
    // std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());
    std::streambuf* prevcoutbuf =
        SerialManager::sharedInstance->changeDefaultCoutBuffer(buffer.rdbuf());

    // BEGIN: Code being tested
    CommandsManager::defaultInstance->shellMode = shellMode;
    SerialManager::sharedInstance->startCommandLog();
    CommandsManager::defaultInstance->processCommand(command);
    SerialManager::sharedInstance->finishCommandLog(CommandsManager::defaultInstance->shellMode);
    // END:   Code being tested

    // Use the string value of buffer to compare against expected output
    std::string text = buffer.str();

    if (CommandsManager::defaultInstance->shellMode)
    {
        text.pop_back(); // remove the last '\n' and '\r' characters printed if
                         // the shellMode is enabled
        text.pop_back();
    }
    else if (text.size() >= 12)
    {
        // 4 begin bytes
        ASSERT_TRUE(text[0] == 0xff && text[1] == 0xfe && text[2] == 0xfd && text[3] == 0xfc);

        // get the size of the expected output (2 bytes), raw bytes of a
        // uint16 and the hash (4 bytes), raw bytes of a uint32

        uint16_t expectedOutputSize = 0;
        expectedOutputSize = (text[5] << 8) | text[4]; // little endian

        // 2 option bytes

        // 4 hash bytes
        uint32_t expectedOutputHash = 0;
        expectedOutputHash =
            (text[11] << 24) | (text[10] << 16) | (text[9] << 8) | text[8]; // little endian

        text.erase(0,
                   12); // remove the size and hash from the expected output

        ASSERT_EQ(std::to_string(text.size()), std::to_string(expectedOutputSize));

        // verify the hash

        uint64_t pseudoHash = 0;

        for (char c : text) pseudoHash = (pseudoHash + c) % 4294967295;

        uint32_t finalPseudoHash = pseudoHash;

        ASSERT_EQ(finalPseudoHash, expectedOutputHash);
    }

    int result = text.compare(expectedOutput);

    // Restore original buffer before exiting
    SerialManager::sharedInstance->changeDefaultCoutBuffer(prevcoutbuf);

    if (result != 0)
    {
        std::cout << "Got shell output: " << text << " expected " << expectedOutput << std::endl;
        std::cout << "shell output size " << text.size() << " expected size "
                  << expectedOutput.size() << std::endl;

        size_t textSize = text.size();
        size_t expectedOutputSize = expectedOutput.size();

        for (size_t i = 0; i < std::max(textSize, expectedOutputSize); i++)
        {
            if (i < textSize && i < expectedOutputSize)
            {
                std::cout << "Binary shell output: " << std::bitset<8>(text.c_str()[i]) << "/"
                          << std::bitset<8>(expectedOutput.c_str()[i]) << std::endl;
                std::cout << "char shell output: " << text.c_str()[i] << "/"
                          << expectedOutput.c_str()[i] << std::endl;
                if (std::bitset<8>(text.c_str()[i]) != std::bitset<8>(expectedOutput.c_str()[i]))
                    std::cout << "Not the same!" << std::endl;
            }
            else if (textSize < expectedOutputSize)
            {
                std::cout << "expected output size is bigger than actual "
                             "output size, char "
                          << std::to_string(i) << " is " << expectedOutput.c_str()[i] << "/"
                          << std::bitset<8>(expectedOutput.c_str()[i]) << std::endl;
            }
            else
            {
                std::cout << "actual output size is bigger than expected "
                             "output size, char "
                          << std::to_string(i) << " is " << text.c_str()[i] << "/"
                          << std::bitset<8>(text.c_str()[i]) << std::endl;
            }
        }

        std::cout << "Arguments: " << command.arguments[0] << " with size "
                  << std::string(command.arguments[0]).size() << "; " << command.arguments[1]
                  << " with size " << std::string(command.arguments[1]).size() << "; "
                  << command.arguments[2] << " with size "
                  << std::string(command.arguments[2]).size() << std::endl;
    }

    SerialManager::sharedInstance->forceFlushBuffers();
    ASSERT_EQ(result, 0);
}
