#pragma once

#include "../CommandsManager.hpp"
#include "../config.h"
#include "SerialManager.hpp"

#include <cstring>
#include <functional>
#include <iostream>
#include <unordered_map>

#define HOSTNAME_MESSAGE "Hostname: " + std::string(HOSTNAME) + "\n"
#define WIFI_MAC_MESSAGE "WiFi Mac Address: " + std::string(MAC_WIFI) + "\n"
#define BLUETOOTH_MAC_MESSAGE "Bluetooth Mac Address: " + std::string(MAC_BLUETOOTH) + "\n"
#define VERSION_MESSAGE "Version: " + std::string(CURRENT_VERSION) + "\n"
#define HARDWARE_MESSAGE "Hardware: " + std::string(HARDWARE_DESCRIPTION) + "\n"
#define INFO_COMMAND_NOT_RECOGNIZED(ARGUMENT)                                                      \
    "Argument " + std::string(ARGUMENT) +                                                          \
        " not recognized (available: hostname, mac, version, hardware)."
#define INFO_COMMAND_NOT_RECOGNIZED_MAC(ARGUMENT)                                                  \
    "Argument " + std::string(ARGUMENT) + " not recognized (available: wifi, bluetooth)."

namespace serialcom
{
    void CommandsManager::processInfoCommandHostname(const Command& command) const
    {
        if (this->shellMode)
        {
            SerialManager::sharedInstance->commandLog(HOSTNAME_MESSAGE);
        }
        else
        {
            SerialManager::sharedInstance->commandLog(HOSTNAME);
            // output the null terminator
            SerialManager::sharedInstance->commandLog("\0");
        }
    }

    void CommandsManager::printWiFiMac() const
    {
        if (this->shellMode)
            SerialManager::sharedInstance->commandLog(WIFI_MAC_MESSAGE); // format it in a fancy way
        else
            SerialManager::sharedInstance->commandLog(MAC_WIFI);
    }

    void CommandsManager::printBluetoothMac() const
    {
        if (this->shellMode)
        {
            SerialManager::sharedInstance->commandLog(
                BLUETOOTH_MAC_MESSAGE
            ); // format it in a fancy way
        }
        else
        {
            SerialManager::sharedInstance->commandLog(MAC_BLUETOOTH);
        }
    }

    void CommandsManager::processInfoCommandMac(const Command& command) const
    {
        std::string potentialSecondParameter = command.arguments[1];
        if (potentialSecondParameter.empty())
        {
            printBluetoothMac();
            printWiFiMac();
        }
        else if (potentialSecondParameter == "wifi")
        {
            printWiFiMac();
        }
        else if (potentialSecondParameter == "bluetooth")
        {
            printBluetoothMac();
        }
        else if (this->shellMode)
        {
            SerialManager::sharedInstance->commandLog(
                INFO_COMMAND_NOT_RECOGNIZED_MAC(potentialSecondParameter)
            );
        }
        else
        {
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
    }

    void CommandsManager::processInfoCommandVersion(const Command& command) const
    {
        if (this->shellMode)
            SerialManager::sharedInstance->commandLog(VERSION_MESSAGE);
        else
            SerialManager::sharedInstance->commandLog(CURRENT_VERSION);
    }

    void CommandsManager::processInfoCommandHardware(const Command& command) const
    {
        if (this->shellMode)
            SerialManager::sharedInstance->commandLog(HARDWARE_MESSAGE);
        else
            SerialManager::sharedInstance->commandLog(HARDWARE_DESCRIPTION);
    }

    void CommandsManager::processInfoCommand(const Command& command) const
    {
        /*
            Version du firmware (2 octets)
            Indicateur du hardware utilisé (8 octets, réservé)
            Nom d'hôte (taille variable, chaîne de caractères terminée par 0x00)
            Adresse MAC Bluetooth (6 octets)
            Adresse MAC WiFi (6 octets)
        */
        std::unordered_map<
            std::string,
            std::function<void(const serialcom::CommandsManager*, const Command&)>>
            commandsMap = {
                {"hostname", &CommandsManager::processInfoCommandHostname},
                {"mac", &CommandsManager::processInfoCommandMac},
                {"version", &CommandsManager::processInfoCommandVersion},
                {"hardware", &CommandsManager::processInfoCommandHardware}
            };

        std::string firstArgument = command.arguments[0];

        if (firstArgument.empty())
        {
            processInfoCommandVersion(command);
            processInfoCommandHardware(command);
            processInfoCommandHostname(command);
            printBluetoothMac();
            printWiFiMac();
            return;
        }
        else if (commandsMap.find(firstArgument) != commandsMap.end())
        {
            commandsMap[firstArgument](this, command);
            return;
        }
        else if (this->shellMode)
        {
            SerialManager::sharedInstance->commandLog(INFO_COMMAND_NOT_RECOGNIZED(firstArgument));
        }
        else
        {
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
    }
} // namespace serialcom
