#pragma once

#include <cstddef>
#include <unordered_map>
#include <optional>
#include <string>

namespace serialcom {
    const size_t MAX_COMMMAND_TYPE_SIZE = 16;
    const size_t MAX_ARGUMENT_SIZE = 512;
    const size_t MAX_COMMAND_ARGUMENTS_COUNT = 3;
    constexpr size_t INPUT_MAX_SIZE = MAX_COMMMAND_TYPE_SIZE + MAX_COMMAND_ARGUMENTS_COUNT * MAX_ARGUMENT_SIZE;

    struct Command {
        enum class CommandType {
            sm,
            console,

            info,
            echo,
            apps,
            files,
            elevate,
            lte,

            ls,
            touch,
            mkdir,
            rm,
            cp,
            mv,
            cat,
            download,
            upload,

            unknown
        };

        // a static unordered map between the raw strings of the command types and the CommandType enum
        
        static std::unordered_map<CommandType, std::string> command_types_raw_strings;
        
        CommandType type;
        char rawCommandType[MAX_COMMMAND_TYPE_SIZE] = {'\0'};
        size_t rawCommandTypeSize = 0;
        char rawInput[INPUT_MAX_SIZE] = {'\0'};
        size_t rawInputSize = 0;

        Command(char (&input)[INPUT_MAX_SIZE]);

        char arguments[MAX_COMMAND_ARGUMENTS_COUNT][MAX_ARGUMENT_SIZE] = {'\0'};
    };
}