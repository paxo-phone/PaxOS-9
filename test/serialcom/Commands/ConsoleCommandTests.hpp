#pragma once

#include "../testCommand.hpp"

#include <Commands/ConsoleCommand.hpp>

using namespace serialcom;

TEST(Commands, CONSOLE_COMMAND)
{
    SerialManager::sharedInstance->changeConsoleLockTo(false);

    testCommand("console lock", true, Command::CommandType::console, CONSOLE_LOCKED);
    ASSERT_TRUE(SerialManager::sharedInstance->getConsoleLockState());

    testCommand("console unlock", true, Command::CommandType::console, CONSOLE_UNLOCKED);
    ASSERT_FALSE(SerialManager::sharedInstance->getConsoleLockState());

    testCommand("console lock", false, Command::CommandType::console, NON_SHELL_MODE_NO_ERROR_CODE);
    ASSERT_TRUE(SerialManager::sharedInstance->getConsoleLockState());

    testCommand(
        "console unlock",
        false,
        Command::CommandType::console,
        NON_SHELL_MODE_NO_ERROR_CODE
    );
    ASSERT_FALSE(SerialManager::sharedInstance->getConsoleLockState());

    testCommand(
        "console wrongArgument",
        true,
        Command::CommandType::console,
        CONSOLE_LOCK_OPTION_NOT_RECOGNIZED("wrongArgument")
    );
    testCommand(
        "console wrongArgument",
        false,
        Command::CommandType::console,
        NON_SHELL_MODE_ERROR_CODE
    );

    SerialManager::sharedInstance->changeConsoleLockTo(false);

    // test the lock effect, it should block any output that is not from the commands and only
    // output them when the unlock is called

    std::stringstream buffer;

    std::streambuf* prevcoutbuf =
        SerialManager::sharedInstance->changeDefaultCoutBuffer(buffer.rdbuf());

    executeCommand("sm disable", false);

    std::string shouldBePrintedMessage = "This message should be printed";

    std::cout << shouldBePrintedMessage << std::flush;

    SerialManager::sharedInstance->forceFlushBuffers();

    ASSERT_EQ(
        buffer.str().substr(12) /* avoid the header things */,
        NON_SHELL_MODE_NO_ERROR_CODE + shouldBePrintedMessage
    );

    buffer.str("");

    SerialManager::sharedInstance->changeConsoleLockTo(true);

    executeCommand("sm disable", false);

    std::cout << shouldBePrintedMessage << std::flush;

    SerialManager::sharedInstance->forceFlushBuffers();

    ASSERT_EQ(buffer.str().substr(12), NON_SHELL_MODE_NO_ERROR_CODE);

    SerialManager::sharedInstance->changeDefaultCoutBuffer(prevcoutbuf);
}
