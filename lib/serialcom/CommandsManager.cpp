#include "CommandsManager.hpp"
#include "SerialManager.hpp"
#include "config.h"
#include <unordered_map>
#include <iostream>
#include <functional>

namespace serialcom {
    const std::shared_ptr<CommandsManager> CommandsManager::defaultInstance = std::make_shared<CommandsManager>();

    void CommandsManager::processCommand(const Command& command) {
        switch (command.type) {
            case Command::CommandType::info:
                processInfoCommand(command);
                break;/*
            case Command::CommandType::echo:
                processEchoCommand(command);
                break;
            case Command::CommandType::apps:
                processAppsCommand(command);
                break;
            case Command::CommandType::files:
                processFilesCommand(command);
                break;
            case Command::CommandType::elevate:
                processElevateCommand(command);
                break;
            case Command::CommandType::lte:
                processLteCommand(command);
                break; */
            default:
                if (this->shellMode) {
                    SerialManager::sharedInstance->commandLog("Unknown command type");
                } else {
                    std::cout << NON_SHELL_MODE_ERROR_CODE << std::flush;
                }
        }
    }
}