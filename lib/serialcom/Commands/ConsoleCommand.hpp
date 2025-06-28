#pragma once

#include "../CommandsManager.hpp"
#include "../config.h"
#include "SerialManager.hpp"

#include <string>

#define CONSOLE_LOCKED "Console locked."
#define CONSOLE_UNLOCKED "Console unlocked."
#define CONSOLE_LOCK_OPTION_NOT_RECOGNIZED(ARGUMENT)                                               \
    "Argument " + std::string(ARGUMENT) + " not recognized (available: lock, unlock)."

namespace serialcom
{
    void CommandsManager::processConsoleCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        if (firstArgument == "lock")
        {
            SerialManager::sharedInstance->changeConsoleLockTo(true);

            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(CONSOLE_LOCKED);
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
        }
        else if (firstArgument == "unlock")
        {
            SerialManager::sharedInstance->changeConsoleLockTo(false);

            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(CONSOLE_UNLOCKED);
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
        }
        else if (this->shellMode)
        {
            SerialManager::sharedInstance->commandLog(
                CONSOLE_LOCK_OPTION_NOT_RECOGNIZED(firstArgument)
            );
        }
        else
        {
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
    }
} // namespace serialcom
