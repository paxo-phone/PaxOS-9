#include "Command.hpp"

#include "Commands/ConsoleCommand.hpp"
#include "Commands/FileCommands.hpp"
#include "Commands/InfoCommand.hpp"
#include "Commands/ShellModeCommand.hpp"

#include <iostream>

namespace serialcom
{
    Command::Command(char (&input)[INPUT_MAX_SIZE])
    {
        bool shellMode = CommandsManager::defaultInstance->shellMode;

        // find the 255,254,253 header in the first bytes of the current_input

        size_t pos = -1;
        for (size_t i = 0; i < INPUT_MAX_SIZE - 3; i++)
        {
            if (input[i] == 0xff && input[i + 1] == 0xfe && input[i + 2] == 0xfd)
            {
                pos = i;
                break;
            }
        }

        if (pos == -1 && !shellMode)
        {
            // check if this is the sm command
            bool found = false;

            for (size_t i = 0; i < INPUT_MAX_SIZE - 2; i++)
            {
                if (input[i] == 's' && input[i + 1] == 'm')
                {
                    pos = i - 3; // to compensate the increment the +3 after that
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                SerialManager::sharedInstance->commandLog(
                    NON_SHELL_MODE_ERROR_CODE + std::string("ERROR: NO HEADER FOUND")
                );
                return;
            }
        }

        size_t current_index = pos + 3;

        memcpy(this->command_id, input + current_index, COMMAND_ID_SIZE);

        current_index += COMMAND_ID_SIZE;

        // skip spaces and null bytes

        while (input[current_index] == ' ' || input[current_index] == '\0') current_index++;

        size_t input_size = 0;
        for (size_t i = 0; i < INPUT_MAX_SIZE; i++)
        {
            if (input[i] == '\0')
            {
                input_size = i;
                break;
            }
            input_size++;
        }

        // size_t current_index = 0;

        // detect the command type knowing that the maximum size that the command
        // type can have is MAX_COMMMAND_TYPE_SIZE char
        // command_type[MAX_COMMMAND_TYPE_SIZE];

        size_t command_size = MAX_COMMMAND_TYPE_SIZE;

        char command_type[MAX_COMMMAND_TYPE_SIZE];

        for (size_t i = 0; i < MAX_COMMMAND_TYPE_SIZE; i++)
        {
            if (input[current_index + i] == ' ' || input[current_index + i] == '\0')
            {
                command_type[i] = '\0';
                command_size = i;
                break;
            }
            command_type[i] = input[current_index + i];
        }

        current_index += command_size;

        // match the command_type with a loop with the CommandType enum but don't
        // forget that the input is a 16 char array

        this->type = CommandType::unknown;

        for (auto const& [key, val] : command_types_raw_strings)
        {
            if (val.size() != command_size)
                continue;

            if (memcmp(command_type, val.c_str(), command_size) == 0)
            {
                this->type = key;
                break;
            }
        }

        if (this->type == CommandType::unknown)
        {
            if (shellMode)
            {
                SerialManager::sharedInstance->commandLog("ERROR: UNKONWN COMMAND TYPE");
            }
            else
            {
                SerialManager::sharedInstance->commandLog(
                    NON_SHELL_MODE_ERROR_CODE + std::string("ERROR: UNKONWN COMMAND TYPE") +
                    std::string(command_type)
                );
            }
            return;
        }

        for (size_t argument_index = 0; argument_index < MAX_COMMAND_ARGUMENTS_COUNT;
             argument_index++)
        {
            // skip spaces
            while (input[current_index] == ' ' && current_index < INPUT_MAX_SIZE - 1)
                current_index++;

            // check if the input has ended
            if (input[current_index] == '\0')
                break;

            bool isUsingQuotes = false;

            if (input[current_index] == '"')
            {
                isUsingQuotes = true;
                current_index++;
            }

            bool isEscaping = false;
            size_t argument_char_index = 0;
            for (size_t i = current_index; i < current_index + MAX_ARGUMENT_SIZE; i++)
            {
                if (input[i] == '\0' || input[i] == '\n')
                {
                    // end of input
                    this->arguments[argument_index][argument_char_index] = '\0';
                    return;
                }
                else if (input[i] == '\\')
                {
                    if (isEscaping)
                    {
                        // escaping backslash
                        isEscaping = false;
                        this->arguments[argument_index][argument_char_index] = '\\';
                        argument_char_index++;
                    }
                    else
                    {
                        // start escaping
                        isEscaping = true;
                    }
                }
                else if (input[i] == ' ')
                {
                    if (isUsingQuotes)
                    {
                        // space and is using quotes
                        this->arguments[argument_index][argument_char_index] = input[i];
                        argument_char_index++;
                        continue;
                    }

                    // not using quotes => need to escape space characters
                    if (isEscaping)
                    {
                        // escaping space
                        isEscaping = false;
                        this->arguments[argument_index][argument_char_index] = ' ';
                        argument_char_index++;
                        continue;
                    }
                    else
                    {
                        // end of argument because of space
                        this->arguments[argument_index][argument_char_index] = '\0';
                        current_index = i + 1;
                        break;
                    }
                }
                else if (input[i] == '"')
                {
                    if (isUsingQuotes)
                    {
                        if (isEscaping)
                        {
                            // escaping quote
                            isEscaping = false;
                            this->arguments[argument_index][argument_char_index] = '"';
                            argument_char_index++;
                        }
                        else
                        {
                            // end of argument because of quote
                            this->arguments[argument_index][argument_char_index] = '\0';
                            current_index = i + 1;
                            break;
                        }
                    }
                    else if (isEscaping)
                    {
                        // escaping quote
                        isEscaping = false;
                        this->arguments[argument_index][argument_char_index] = '"';
                        argument_char_index++;
                    }
                    else
                    {
                        if (shellMode)
                        {
                            SerialManager::sharedInstance->commandLog(
                                "ERROR: UNEXPECTED QUOTE CHARACTER AT "
                                "POSITION " +
                                std::to_string(i)
                            );
                        }
                        else
                        {
                            SerialManager::sharedInstance->commandLog(
                                NON_SHELL_MODE_ERROR_CODE +
                                std::string(
                                    "ERROR: UNEXPECTED QUOTE CHARACTER "
                                    "AT POSITION "
                                ) +
                                std::to_string(i)
                            );
                        }
                        this->type = CommandType::unknown;
                        for (size_t j = 0; j < MAX_COMMAND_ARGUMENTS_COUNT; j++)
                            this->arguments[j][0] = '\0';
                        return;
                    }
                }
                else
                {
                    if (isEscaping)
                    {
                        // escaping character
                        isEscaping = false;
                        this->arguments[argument_index][argument_char_index] = '\\';
                        argument_char_index++;
                    }
                    this->arguments[argument_index][argument_char_index] = input[i];
                    argument_char_index++;
                }
            }
        }
    }

    std::unordered_map<Command::CommandType, std::string> Command::command_types_raw_strings{
        {Command::CommandType::sm, "sm"},
        {Command::CommandType::console, "console"},

        {Command::CommandType::info, "info"},
        {Command::CommandType::echo, "echo"},
        {Command::CommandType::apps, "apps"},
        {Command::CommandType::files, "files"},
        {Command::CommandType::elevate, "elevate"},
        {Command::CommandType::lte, "lte"},

        {Command::CommandType::ls, "ls"},
        {Command::CommandType::touch, "touch"},
        {Command::CommandType::mkdir, "mkdir"},
        {Command::CommandType::rm, "rm"},
        {Command::CommandType::cp, "cp"},
        {Command::CommandType::mv, "mv"},
        {Command::CommandType::cat, "cat"},
        {Command::CommandType::download, "download"},
        {Command::CommandType::upload, "upload"}
    };
} // namespace serialcom
