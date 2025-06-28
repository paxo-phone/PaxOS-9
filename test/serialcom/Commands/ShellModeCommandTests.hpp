#pragma once

#include "../testCommand.hpp"

#include <Commands/ShellModeCommand.hpp>

using namespace serialcom;

TEST(Commands, SHELL_MODE_COMMAND)
{
    CommandsManager::defaultInstance->shellMode = true;

    testCommand("sm disable", true, Command::CommandType::sm, NON_SHELL_MODE_NO_ERROR_CODE);
    ASSERT_FALSE(CommandsManager::defaultInstance->shellMode);

    testCommand("sm enable", false, Command::CommandType::sm, SHELL_MODE_ENABLED);
    ASSERT_TRUE(CommandsManager::defaultInstance->shellMode);

    testCommand(
        "sm wrongArgument",
        true,
        Command::CommandType::sm,
        SHELL_MODE_OPTION_NOT_RECOGNIZED("wrongArgument")
    );
    testCommand("sm wrongArgument", false, Command::CommandType::sm, NON_SHELL_MODE_ERROR_CODE);
}
