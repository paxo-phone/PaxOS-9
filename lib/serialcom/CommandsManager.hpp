#pragma once

#include "Command.hpp"
#include <shared_mutex>

namespace serialcom {
    class CommandsManager {
        public:
            CommandsManager() = default;

            static const std::shared_ptr<CommandsManager> defaultInstance;

            void processCommand(const Command& command);

            bool shellMode = true;

        private:
            void processInfoCommand(const Command& command) const;
            void processInfoCommandHostname(const Command& command) const;
            void printWiFiMac() const;
            void printBluetoothMac() const;
            void processInfoCommandMac(const Command& command) const;
            void processInfoCommandVersion(const Command& command) const;
            void processInfoCommandHardware(const Command& command) const;


            void processEchoCommand(const Command& command) const;
            void processAppsCommand(const Command& command) const;
            void processFilesCommand(const Command& command) const;
            void processElevateCommand(const Command& command) const;
            void processLteCommand(const Command& command) const;
    };
}