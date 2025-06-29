#pragma once

#include "Command.hpp"

#include <memory>

namespace serialcom
{
    class CommandsManager
    {
      public:
        CommandsManager() = default;

        static const std::shared_ptr<CommandsManager> defaultInstance;

        void processCommand(const Command& command1);

        bool shellMode = true;

      private:
        void processSMCommand(const Command& command);

        void processConsoleCommand(const Command& command) const;

        void processInfoCommand(const Command& command) const;
        void processInfoCommandHostname(const Command& command) const;
        void printWiFiMac() const;
        void printBluetoothMac() const;
        void processInfoCommandMac(const Command& command) const;
        void processInfoCommandVersion(const Command& command) const;
        void processInfoCommandHardware(const Command& command) const;

        void processLSCommand(const Command& command) const;
        void processTouchCommand(const Command& command) const;
        void processMKDIRCommand(const Command& command) const;
        void processRMCommand(const Command& command) const;
        void processCPCommand(const Command& command) const;
        void processMVCommand(const Command& command) const;
        void processCATCommand(const Command& command) const;
        void processDownloadCommand(const Command& command) const;
        void processUploadCommand(const Command& command) const;

        void processEchoCommand(const Command& command) const;
        void processAppsCommand(const Command& command) const;
        void processFilesCommand(const Command& command) const;
        void processElevateCommand(const Command& command) const;
        void processLteCommand(const Command& command) const;
    };
} // namespace serialcom
