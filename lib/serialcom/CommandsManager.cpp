#include "CommandsManager.hpp"

#include "SerialManager.hpp"
#include "config.h"

#include <functional>
#include <iostream>
#include <unordered_map>

namespace serialcom
{
    const std::shared_ptr<CommandsManager> CommandsManager::defaultInstance =
        std::make_shared<CommandsManager>();

    void CommandsManager::processCommand(const Command& command)
    {
        switch (command.type)
        {
        case Command::CommandType::sm:
            processSMCommand(command);
            break;
        case Command::CommandType::console:
            processConsoleCommand(command);
            break;
        case Command::CommandType::info:
            processInfoCommand(command);
            break;
        case Command::CommandType::ls:
            processLSCommand(command);
            break;
        case Command::CommandType::touch:
            processTouchCommand(command);
            break;
        case Command::CommandType::mkdir:
            processMKDIRCommand(command);
            break;
        case Command::CommandType::rm:
            processRMCommand(command);
            break;
        case Command::CommandType::cp:
            processCPCommand(command);
            break;
        case Command::CommandType::mv:
            processMVCommand(command);
            break;
        case Command::CommandType::cat:
            processCATCommand(command);
            break;
        case Command::CommandType::download:
            processDownloadCommand(command);
            break;
        case Command::CommandType::upload:
            processUploadCommand(command);
            break;
        /*
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
            if (this->shellMode)
            {
                SerialManager::sharedInstance->commandLog("Unknown command type");
            }
            else
            {
                SerialManager::sharedInstance->commandLog(
                    NON_SHELL_MODE_ERROR_CODE + std::string("Unknown command type")
                );
            }
        }
    }
} // namespace serialcom
