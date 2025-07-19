#pragma once

#include "../CommandsManager.hpp"
#include "../config.h"
#include "SerialManager.hpp"

#include <string>

#define SHELL_MODE_ENABLED "Shell mode enabled."
#define SHELL_MODE_OPTION_NOT_RECOGNIZED(ARGUMENT)                                                 \
    "Argument " + std::string(ARGUMENT) + " not recognized (available: enable, disable)."

namespace serialcom
{
    void CommandsManager::processSMCommand(const Command& command)
    {
        std::string firstArgument = command.arguments[0];

        if (firstArgument == "enable")
        {
            this->shellMode = true;
            SerialManager::sharedInstance->commandLog(SHELL_MODE_ENABLED);
        }
        else if (firstArgument == "disable")
        {
            this->shellMode = false;
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
        }
        else if (this->shellMode)
        {
            SerialManager::sharedInstance->commandLog(
                SHELL_MODE_OPTION_NOT_RECOGNIZED(firstArgument)
            );
        }
        else
        {
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
    }
} // namespace serialcom
