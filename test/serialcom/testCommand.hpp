#pragma once

#include <gtest/gtest.h>
#include <bitset>
#include <CommandsManager.hpp>
#include <Command.hpp>
#include <algorithm>

using namespace serialcom;

void testCommand(std::string commandText, bool shellMode, Command::CommandType expectedCommandType, std::string expectedOutput)
{
    char info_input[serialcom::INPUT_MAX_SIZE] = {'\0'};
    strcpy(info_input, commandText.c_str());
    Command command = Command(info_input);
    ASSERT_EQ(command.type, expectedCommandType);

    std::stringstream buffer;
    // Redirect std::cout to buffer
    std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());

    // BEGIN: Code being tested
    CommandsManager::defaultInstance->shellMode = shellMode;
    CommandsManager::defaultInstance->processCommand(command);
    // END:   Code being tested

    // Use the string value of buffer to compare against expected output
    std::string text = buffer.str();
    int result = text.compare(expectedOutput);

    // Restore original buffer before exiting
    std::cout.rdbuf(prevcoutbuf);


   if (result != 0)
   {
         std::cout << "Got shell output: " << text << " expected " << expectedOutput << std::endl;
         std::cout << "shell output size " << text.size() << " expected size " << expectedOutput.size() << std::endl; 
         /*
         for (size_t i = 0; i < std::min(text.size(), expectedOutput.size()); i++) {
              std::cout << "Binary shell output: " << std::bitset<8>(text.c_str()[i]) << "/" <<  std::bitset<8>(expectedOutput.c_str()[i]) << std::endl;
              std::cout << "char shell output: " << text.c_str()[i] << "/" <<  expectedOutput.c_str()[i] << std::endl;
              if (std::bitset<8>(text.c_str()[i]) != std::bitset<8>(expectedOutput.c_str()[i]))
              {
                std::cout << "Not the same!" << std::endl;
              }
         }*/
   }

    ASSERT_EQ(result, 0);
}