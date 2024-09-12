#pragma once

#include <gtest/gtest.h>
#include <bitset>
#include <CommandsManager.hpp>
#include <Command.hpp>
#include <algorithm>

using namespace serialcom;

void executeCommand(std::string commandText, bool shellMode)
{
    char info_input[serialcom::INPUT_MAX_SIZE] = {'\0'};
    strcpy(info_input, commandText.c_str());
    Command command = Command(info_input);
    CommandsManager::defaultInstance->shellMode = shellMode;
    CommandsManager::defaultInstance->processCommand(command);
}

void testCommand(std::string commandText, bool shellMode, Command::CommandType expectedCommandType, std::string expectedOutput)
{
    char info_input[serialcom::INPUT_MAX_SIZE] = {'\0'};
    strcpy(info_input, commandText.c_str());
    Command command = Command(info_input);
    ASSERT_EQ(command.type, expectedCommandType);

    std::stringstream buffer;
    // Redirect std::cout to buffer
    //std::streambuf* prevcoutbuf = std::cout.rdbuf(buffer.rdbuf());
    std::streambuf* prevcoutbuf = SerialManager::sharedInstance->changeDefaultCoutBuffer(buffer.rdbuf());

    // BEGIN: Code being tested
    CommandsManager::defaultInstance->shellMode = shellMode;
    CommandsManager::defaultInstance->processCommand(command);
    // END:   Code being tested

    // Use the string value of buffer to compare against expected output
    std::string text = buffer.str();

    if (CommandsManager::defaultInstance->shellMode)
      text.pop_back(); // remove the last '\n' character printed if the shellMode is enabled

    int result = text.compare(expectedOutput);

    // Restore original buffer before exiting
    SerialManager::sharedInstance->changeDefaultCoutBuffer(prevcoutbuf);

   if (result != 0)
   {
        std::cout << "Got shell output: " << text << " expected " << expectedOutput << std::endl;
        std::cout << "shell output size " << text.size() << " expected size " << expectedOutput.size() << std::endl; 
         
        size_t textSize = text.size();
        size_t expectedOutputSize = expectedOutput.size();

         for (size_t i = 0; i < std::max(textSize, expectedOutputSize); i++) {
              if (i < textSize && i < expectedOutputSize)
              {
                  std::cout << "Binary shell output: " << std::bitset<8>(text.c_str()[i]) << "/" <<  std::bitset<8>(expectedOutput.c_str()[i]) << std::endl;
                  std::cout << "char shell output: " << text.c_str()[i] << "/" <<  expectedOutput.c_str()[i] << std::endl;
                  if (std::bitset<8>(text.c_str()[i]) != std::bitset<8>(expectedOutput.c_str()[i]))
                  {
                    std::cout << "Not the same!" << std::endl;
                  }
              } else
              {
                  if (textSize < expectedOutputSize)
                  {
                      std::cout << "expected output size is bigger than actual output size, char " << std::to_string(i) << " is " << expectedOutput.c_str()[i] << "/" << std::bitset<8>(expectedOutput.c_str()[i]) << std::endl;
                  } else
                  {
                      std::cout << "actual output size is bigger than expected output size, char " << std::to_string(i) << " is " << text.c_str()[i] << "/" << std::bitset<8>(text.c_str()[i]) << std::endl;
                  }
              }
         }

         std::cout << "Arguments: " << command.arguments[0] << " with size " << std::string(command.arguments[0]).size() << "; " << command.arguments[1] << " with size " << std::string(command.arguments[1]).size() << "; " << command.arguments[2] << " with size " << std::string(command.arguments[2]).size() << std::endl;
   }

    SerialManager::sharedInstance->forceFlushBuffers();
    ASSERT_EQ(result, 0);
}