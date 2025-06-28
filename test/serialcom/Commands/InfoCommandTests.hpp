#pragma once

#include "../testCommand.hpp"

#include <Commands/InfoCommand.hpp>

using namespace serialcom;

TEST(Commands, INFO_COMMAND)
{
    testCommand(
        "info",
        true,
        Command::CommandType::info,
        VERSION_MESSAGE + HARDWARE_MESSAGE + HOSTNAME_MESSAGE + BLUETOOTH_MAC_MESSAGE +
            WIFI_MAC_MESSAGE
    );

    char hostname[512] = {'\0'};
    strcpy(hostname, HOSTNAME);
    // get the lenght of the actual hostname, including the null terminator
    size_t hostnameLength = strlen(hostname);
    testCommand(
        "info",
        false,
        Command::CommandType::info,
        std::string(CURRENT_VERSION) + std::string(HARDWARE_DESCRIPTION) +
            std::string(hostname, hostnameLength) + std::string(MAC_BLUETOOTH) +
            std::string(MAC_WIFI)
    );
    testCommand("info version", true, Command::CommandType::info, VERSION_MESSAGE);
    testCommand("info version", false, Command::CommandType::info, std::string(CURRENT_VERSION));
    testCommand("info hardware", true, Command::CommandType::info, HARDWARE_MESSAGE);
    testCommand(
        "info hardware",
        false,
        Command::CommandType::info,
        std::string(HARDWARE_DESCRIPTION)
    );
    testCommand("info hostname", true, Command::CommandType::info, HOSTNAME_MESSAGE);
    testCommand("info hostname", false, Command::CommandType::info, std::string(HOSTNAME));
    testCommand(
        "info mac",
        true,
        Command::CommandType::info,
        BLUETOOTH_MAC_MESSAGE + WIFI_MAC_MESSAGE
    );
    testCommand(
        "info mac",
        false,
        Command::CommandType::info,
        std::string(MAC_BLUETOOTH) + std::string(MAC_WIFI)
    );
    testCommand("info mac wifi", true, Command::CommandType::info, WIFI_MAC_MESSAGE);
    testCommand("info mac wifi", false, Command::CommandType::info, std::string(MAC_WIFI));
    testCommand("info mac bluetooth", true, Command::CommandType::info, BLUETOOTH_MAC_MESSAGE);
    testCommand(
        "info mac bluetooth",
        false,
        Command::CommandType::info,
        std::string(MAC_BLUETOOTH)
    );

    // test invalid command

    testCommand(
        "info invalid",
        true,
        Command::CommandType::info,
        INFO_COMMAND_NOT_RECOGNIZED("invalid")
    );
    testCommand("info invalid", false, Command::CommandType::info, NON_SHELL_MODE_ERROR_CODE);
    testCommand(
        "info mac invalid",
        true,
        Command::CommandType::info,
        INFO_COMMAND_NOT_RECOGNIZED_MAC("invalid")
    );
    testCommand("info mac invalid", false, Command::CommandType::info, NON_SHELL_MODE_ERROR_CODE);
}
