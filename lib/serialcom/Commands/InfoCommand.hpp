#include "../config.h"
#include "../CommandsManager.hpp"
#include "SerialManager.hpp"
#include <cstring>
#include <unordered_map>
#include <iostream>
#include <functional>

namespace serialcom {
    void CommandsManager::processInfoCommandHostname(const Command& command) const {
        if (this->shellMode) {
            SerialManager::sharedInstance->commandLog("Hostname: " + std::string(HOSTNAME));
        } else {
            SerialManager::sharedInstance->commandLog(HOSTNAME);
            // output the null terminator
            SerialManager::sharedInstance->commandLog("\0");
        }
    }


    void CommandsManager::printWiFiMac() const {
        if (this->shellMode) {
            SerialManager::sharedInstance->commandLog("WiFi Mac Address: " + std::string(MAC_WIFI)); // format it in a fancy way
        } else {
            SerialManager::sharedInstance->commandLog(MAC_WIFI);
        }
    }

    void CommandsManager::printBluetoothMac() const {
        if (this->shellMode) {
            SerialManager::sharedInstance->commandLog("Bluetooth Mac Address: " + std::string(MAC_BLUETOOTH)); // format it in a fancy way
        } else {
            SerialManager::sharedInstance->commandLog(MAC_BLUETOOTH);
        }
    }

    void CommandsManager::processInfoCommandMac(const Command& command) const {
        std::string potentialSecondParameter = command.arguments[1];
        if (potentialSecondParameter.empty()) {
            printBluetoothMac();
            printWiFiMac();
        } else if (potentialSecondParameter == "wifi") {
            printWiFiMac();
        } else if (potentialSecondParameter == "bluetooth") {
            printBluetoothMac();
        } else {
            if (this->shellMode) {
                SerialManager::sharedInstance->commandLog("Argument " + potentialSecondParameter + " not recognized (available: wifi, bluetooth)");
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        }
    }

    void CommandsManager::processInfoCommandVersion(const Command& command) const {
        if (this->shellMode) {
            SerialManager::sharedInstance->commandLog("Version: " + std::string(CURRENT_VERSION));
        } else {
            SerialManager::sharedInstance->commandLog(CURRENT_VERSION);
        }
    }

    void CommandsManager::processInfoCommandHardware(const Command& command) const {
        if (this->shellMode) {
            SerialManager::sharedInstance->commandLog("Hardware: " + std::string(HARDWARE_DESCRIPTION));
        } else {
            SerialManager::sharedInstance->commandLog(HARDWARE_DESCRIPTION);
        }
    }

    void CommandsManager::processInfoCommand(const Command& command) const {
        /*
            Version du firmware (2 octets)
            Indicateur du hardware utilisé (8 octets, réservé)
            Nom d'hôte (taille variable, chaîne de caractères terminée par 0x00)
            Adresse MAC Bluetooth (6 octets)
            Adresse MAC WiFi (6 octets)
        */
        std::unordered_map<std::string, std::function<void(const serialcom::CommandsManager*, const Command&)>> commandsMap = {
            {"hostname", &CommandsManager::processInfoCommandHostname},
            {"mac", &CommandsManager::processInfoCommandMac},
            {"version", &CommandsManager::processInfoCommandVersion},
            {"hardware", &CommandsManager::processInfoCommandHardware}
        };

        std::string firstArgument = command.arguments[0];

        if (firstArgument.empty()) {
            processInfoCommandVersion(command);
            processInfoCommandHardware(command);
            processInfoCommandHostname(command);
            printBluetoothMac();
            printWiFiMac();
            return;
        } else if (commandsMap.find(firstArgument) != commandsMap.end()) {
            commandsMap[firstArgument](this, command);
            return;
        } else {
            if (this->shellMode) {
                SerialManager::sharedInstance->commandLog("Argument " + firstArgument + " not recognized (available: hostname, mac, version, hardware)");
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        }
    }
}