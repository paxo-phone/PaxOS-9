#pragma once

#include <gtest/gtest.h>

using namespace serialcom;

TEST(Commands, COMMAND_STRUCTURE)
{
    
    // tests for the "Command" structure (no precise command type is tested here, only the constructor and the structure itself)

    // test the constructor
    char input[INPUT_MAX_SIZE] = {'\0'};
    Command command = Command(input);
    ASSERT_EQ(command.type, Command::CommandType::unknown);


    // test the arguments
    for (size_t i = 0; i < MAX_COMMAND_ARGUMENTS_COUNT; i++)
    {
        for (size_t j = 0; j < MAX_ARGUMENT_SIZE; j++)
        {
            ASSERT_EQ(command.arguments[i][j], '\0');
        }
    }
    

    // test the command type strings
    ASSERT_EQ(Command::command_types_raw_strings[Command::CommandType::info], "info");
    ASSERT_EQ(Command::command_types_raw_strings[Command::CommandType::echo], "echo");
    ASSERT_EQ(Command::command_types_raw_strings[Command::CommandType::apps], "apps");
    ASSERT_EQ(Command::command_types_raw_strings[Command::CommandType::files], "files");
    ASSERT_EQ(Command::command_types_raw_strings[Command::CommandType::elevate], "elevate");
    ASSERT_EQ(Command::command_types_raw_strings[Command::CommandType::lte], "lte");

    
    // test the unknown command type
    ASSERT_EQ(Command::command_types_raw_strings[Command::CommandType::unknown], "");

    
    // test the input max size
    ASSERT_EQ(INPUT_MAX_SIZE, MAX_COMMMAND_TYPE_SIZE + MAX_COMMAND_ARGUMENTS_COUNT * MAX_ARGUMENT_SIZE);


    // test all the stuff for an example command
    char input2[INPUT_MAX_SIZE] = {'\0'};
    strcpy(input2, "info \"\\\"hello world\\\"\" hello\\ world 1234");

    Command command2 = Command(input2);
    ASSERT_EQ(command2.type, Command::CommandType::info);
    ASSERT_EQ(strcmp(command2.arguments[0], "\"hello world\""), 0);
    ASSERT_EQ(strcmp(command2.arguments[1], "hello world"), 0);
    ASSERT_EQ(strcmp(command2.arguments[2], "1234"), 0);
}