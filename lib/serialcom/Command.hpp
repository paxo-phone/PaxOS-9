#pragma once

#include <cstddef>
#include <unordered_map>
#include <optional>

namespace serialcom {
    const size_t MAX_COMMMAND_TYPE_SIZE = 16;
    const size_t MAX_ARGUMENT_SIZE = 512;
    const size_t MAX_COMMAND_ARGUMENTS_COUNT = 3;
    constexpr size_t INPUT_MAX_SIZE = MAX_COMMMAND_TYPE_SIZE + MAX_COMMAND_ARGUMENTS_COUNT * MAX_ARGUMENT_SIZE;

    struct Command {
        enum class CommandType {
            info,
            echo,
            apps,
            files,
            elevate,
            lte,

            unknown
        };

        // a static unordered map between the raw strings of the command types and the CommandType enum
        
        static std::unordered_map<CommandType, std::string> command_types_raw_strings;
        
        CommandType type;

        Command(char (&input)[INPUT_MAX_SIZE]);

        char arguments[MAX_COMMAND_ARGUMENTS_COUNT][MAX_ARGUMENT_SIZE] = {'\0'};
    };
}