#pragma once

#include "../testCommand.hpp"

using namespace serialcom;

TEST(Commands, INFO_COMMAND)
{
    testCommand("info", true, Command::CommandType::info, "Version: " + std::string(CURRENT_VERSION) + "\n" + "Hardware: " + std::string(HARDWARE_DESCRIPTION) + "\n" + "Hostname: " + std::string(HOSTNAME) + "\n" + "Bluetooth Mac Address: " + std::string(MAC_BLUETOOTH) + "\n" + "WiFi Mac Address: " + std::string(MAC_WIFI) + "\n");
    
    char hostname[512] = {'\0'};
    strcpy(hostname, HOSTNAME);
    // get the lenght of the actual hostname, including the null terminator
    size_t hostnameLength = strlen(hostname) + 1;
    testCommand("info", false, Command::CommandType::info, std::string(CURRENT_VERSION) + std::string(HARDWARE_DESCRIPTION) + std::string(hostname, hostnameLength) + std::string(MAC_BLUETOOTH) + std::string(MAC_WIFI));
    testCommand("info version", true, Command::CommandType::info, "Version: " + std::string(CURRENT_VERSION) + "\n");
    testCommand("info version", false, Command::CommandType::info, std::string(CURRENT_VERSION));
    testCommand("info hardware", true, Command::CommandType::info, "Hardware: " + std::string(HARDWARE_DESCRIPTION) + "\n");
    testCommand("info hardware", false, Command::CommandType::info, std::string(HARDWARE_DESCRIPTION));
    testCommand("info hostname", true, Command::CommandType::info, "Hostname: " + std::string(HOSTNAME) + "\n");
    testCommand("info hostname", false, Command::CommandType::info, std::string(HOSTNAME) + '\0');
    testCommand("info mac", true, Command::CommandType::info, "Bluetooth Mac Address: " + std::string(MAC_BLUETOOTH) + "\n" + "WiFi Mac Address: " + std::string(MAC_WIFI) + "\n");
    testCommand("info mac", false, Command::CommandType::info, std::string(MAC_BLUETOOTH) + std::string(MAC_WIFI));
    testCommand("info mac wifi", true, Command::CommandType::info, "WiFi Mac Address: " + std::string(MAC_WIFI) + "\n");
    testCommand("info mac wifi", false, Command::CommandType::info, std::string(MAC_WIFI));
    testCommand("info mac bluetooth", true, Command::CommandType::info, "Bluetooth Mac Address: " + std::string(MAC_BLUETOOTH) + "\n");
    testCommand("info mac bluetooth", false, Command::CommandType::info, std::string(MAC_BLUETOOTH));

    // test invalid command

    testCommand("info invalid", true, Command::CommandType::info, "Argument invalid not recognized (available: hostname, mac, version, hardware)\n");
    testCommand("info invalid", false, Command::CommandType::info, NON_SHELL_MODE_ERROR_CODE);
    testCommand("info mac invalid", true, Command::CommandType::info, "Argument invalid not recognized (available: wifi, bluetooth)\n");
    testCommand("info mac invalid", false, Command::CommandType::info, NON_SHELL_MODE_ERROR_CODE);
}