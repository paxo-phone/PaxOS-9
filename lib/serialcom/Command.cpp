#include "Command.hpp"
#include "Commands/InfoCommand.hpp"
#include <iostream>

namespace serialcom {
    Command::Command(char (&input)[INPUT_MAX_SIZE]) {
        // detect the command type knowing that the maximum size that the command type can have is MAX_COMMMAND_TYPE_SIZE
        char command_type[MAX_COMMMAND_TYPE_SIZE];

        size_t command_size = MAX_COMMMAND_TYPE_SIZE;

        for (size_t i = 0; i < MAX_COMMMAND_TYPE_SIZE; i++) {
            if (input[i] == ' ' || input[i] == '\0') {
                command_type[i] = '\0';
                command_size = i;
                break;
            }
            command_type[i] = input[i];
        }

        // match the command_type with a loop with the CommandType enum but don't forget that the input is a 16 char array

        this->type = CommandType::unknown;
        for (auto const& [key, val] : command_types_raw_strings) {
            if (val.size() == 0) {
                continue;
            }
            if (MAX_COMMMAND_TYPE_SIZE < val.size()) {
                SerialManager::sharedInstance->log("WARNING: COMMAND TYPE " + val + " HAS A GREATER SIZE THAN THE MAXIMUM COMMAND TYPE SIZE");
                continue;
            }

            bool broken = false;

            for(size_t i = 0; i < val.size(); i++) {
                if (command_type[i] != val[i]) {
                    broken = true;
                    break;
                }
            }
            if (!broken)
            {
                this->type = key;
                break;
            }
        }

        if (type == CommandType::unknown) {
            SerialManager::sharedInstance->log("ERROR: UNKONWN COMMAND TYPE");
            return;
        }

        size_t current_index = command_size;

        for (size_t argument_index = 0; argument_index < MAX_COMMAND_ARGUMENTS_COUNT; argument_index++) {
            // skip spaces
            while (input[current_index] == ' ' && current_index < INPUT_MAX_SIZE - 1) {
                current_index++;
            }

            // check if the input has ended
            if (input[current_index] == '\0') {
                break;
            }

            bool isUsingQuotes = false;

            if (input[current_index] == '"') {
                isUsingQuotes = true;
                current_index++;
            }

            bool isEscaping = false;
            size_t argument_char_index = 0;
            for (size_t i = current_index; i < current_index + MAX_ARGUMENT_SIZE; i++)
            {
                if (input[i] == '\0' || input[i] == '\n') {
                    if (isEscaping) {
                        // escaping end of input
                        isEscaping = false;
                        this->arguments[argument_index][argument_char_index] = input[i];
                        current_index = i + 1;
                        break;
                    } else {
                        // end of input
                        this->arguments[argument_index][argument_char_index] = '\0';
                        current_index = i + 1;
                        break;
                    }
                } else if (input[i] == '\\') {
                    if (isEscaping) {
                        // escaping backslash
                        isEscaping = false;
                        this->arguments[argument_index][argument_char_index] = '\\';
                        argument_char_index++;
                    } else {
                        // start escaping
                        isEscaping = true;
                    }
                } else if (input[i] == ' ') {
                    if (isUsingQuotes) {
                        // space and is using quotes
                        this->arguments[argument_index][argument_char_index] = input[i];
                        argument_char_index++;
                        continue;
                    }

                    // not using quotes => need to escape space characters
                    if (isEscaping) {
                        // escaping space
                        isEscaping = false;
                        this->arguments[argument_index][argument_char_index] = ' ';
                        argument_char_index++;
                        continue;
                    } else {
                        // end of argument because of space
                        this->arguments[argument_index][argument_char_index] = '\0';
                        current_index = i + 1;
                        break;
                    }
                } else if (input[i] == '"') {
                    if (isUsingQuotes) {
                            if (isEscaping) {
                                // escaping quote
                                isEscaping = false;
                                this->arguments[argument_index][argument_char_index] = '"';
                                argument_char_index++;
                            } else {
                                // end of argument because of quote
                                this->arguments[argument_index][argument_char_index] = '\0';
                                current_index = i + 1;
                                break;
                            }
                    } else {
                        if (isEscaping) {
                            // escaping quote
                            isEscaping = false;
                            this->arguments[argument_index][argument_char_index] = '"';
                            argument_char_index++;
                        } else {
                            SerialManager::sharedInstance->log("ERROR: UNEXPECTED QUOTE CHARACTER AT POSITION " + std::to_string(i));
                            this->type = CommandType::unknown;
                            for (size_t j = 0; j < MAX_COMMAND_ARGUMENTS_COUNT; j++)
                            {
                                this->arguments[j][0] = '\0';
                            }
                            return;
                        }
                    }
                } else {
                    if (isEscaping) {
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

    std::unordered_map<Command::CommandType, std::string> Command::command_types_raw_strings {
        {Command::CommandType::info, "info"},
        {Command::CommandType::echo, "echo"},
        {Command::CommandType::apps, "apps"},
        {Command::CommandType::files, "files"},
        {Command::CommandType::elevate, "elevate"},
        {Command::CommandType::lte, "lte"},
    };
}

