#pragma once

#include "../testCommand.hpp"

using namespace serialcom;

TEST(Commands, FILE_COMMANDS)
{
    storage::init();

    /*
    ls, ok
    touch, ok
    mkdir, ok
    rm, ok
    cp, -> dirs
    mv, -> dirs
    cat, -> not implemented
    download, ok
    upload, ok
    */

    /*
    current state of /:
    /FILE_COMMANDS_TEST
    ...
    */
    executeCommand("rm /FILE_COMMANDS_TEST", true);
    executeCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE", true);

    testCommand("mkdir /FILE_COMMANDS_TEST", true, Command::CommandType::mkdir, "Directory /FILE_COMMANDS_TEST created.\n");
    testCommand("mkdir /FILE_COMMANDS_TEST_NON_SHELL_MODE", false, Command::CommandType::mkdir, NON_SHELL_MODE_NO_ERROR_CODE);

    

    testCommand("mkdir /FILE_COMMANDS_TEST", true, Command::CommandType::mkdir, "Directory /FILE_COMMANDS_TEST already exists.\n");
    testCommand("mkdir /FILE_COMMANDS_TEST_NON_SHELL_MODE", false, Command::CommandType::mkdir, NON_SHELL_MODE_ERROR_CODE);

    //testCommand("mkdir FILE_COMMANDS_TESTç*\"+_=?%:\n\t\r", true, Command::CommandType::mkdir, "Error creating directory FILE_COMMANDS_TESTç*\"+_=?%:\n\t\r");
    //testCommand("mkdir FILE_COMMANDS_TEST_NON_SHELL_MODEç*\"+_=?%:\n\t\r", false, Command::CommandType::mkdir, NON_SHELL_MODE_ERROR_CODE);


    /*
    current state of /FILE_COMMANDS_TEST:
    (empty)
    */

    testCommand("ls /FILE_COMMANDS_TEST", true, Command::CommandType::ls, "Files and directories in /FILE_COMMANDS_TEST:\n");
    testCommand("ls /FILE_COMMANDS_TEST_NON_SHELL_MODE", false, Command::CommandType::ls, "[]");

    testCommand("ls /FILE_COMMANDS_TEST1", true, Command::CommandType::ls, "Directory /FILE_COMMANDS_TEST1 does not exist.\n");
    testCommand("ls /FILE_COMMANDS_TEST_NON_SHELL_MODE1", false, Command::CommandType::ls, NON_SHELL_MODE_ERROR_CODE);

    executeCommand("rm /FILE_COMMANDS_TEST/file.txt", true);
    executeCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt", true);

    testCommand("touch /FILE_COMMANDS_TEST/file.txt", true, Command::CommandType::touch, "File /FILE_COMMANDS_TEST/file.txt created.\n");
    testCommand("touch /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt", false, Command::CommandType::touch, NON_SHELL_MODE_NO_ERROR_CODE);

    testCommand("touch /FILE_COMMANDS_TEST/file.txt", true, Command::CommandType::touch, "File /FILE_COMMANDS_TEST/file.txt already exists.\n");
    testCommand("touch /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt", false, Command::CommandType::touch, NON_SHELL_MODE_ERROR_CODE);

    //testCommand("touch FILE_COMMANDS_TESTç*\"+_=?%:\n\t\r/file.txt", true, Command::CommandType::touch, "Error creating file /FILE_COMMANDS_TESTç*\"+_=?%:\n\t\r/file.txt");
    //testCommand("touch FILE_COMMANDS_TEST_NON_SHELL_MODEç*\"+_=?%:\n\t\r/file.txt", false, Command::CommandType::touch, NON_SHELL_MODE_ERROR_CODE);

    /* 
    current state of /FILE_COMMANDS_TEST:
    file.txt
    */

    testCommand("ls /FILE_COMMANDS_TEST", true, Command::CommandType::ls, "Files and directories in /FILE_COMMANDS_TEST:\nfile.txt\n");
    testCommand("ls /FILE_COMMANDS_TEST_NON_SHELL_MODE", false, Command::CommandType::ls, "[\"file.txt\"]");

    testCommand("rm /FILE_COMMANDS_TEST/file.txt", true, Command::CommandType::rm, "File or directory /FILE_COMMANDS_TEST/file.txt removed.\n");
    testCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt", false, Command::CommandType::rm, NON_SHELL_MODE_NO_ERROR_CODE);

    testCommand("rm /FILE_COMMANDS_TEST/file.txt", true, Command::CommandType::rm, "Error: /FILE_COMMANDS_TEST/file.txt does not exist.\n");
    testCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt", false, Command::CommandType::rm, NON_SHELL_MODE_ERROR_CODE);

    testCommand("ls /FILE_COMMANDS_TEST", true, Command::CommandType::ls, "Files and directories in /FILE_COMMANDS_TEST:\n");
    testCommand("ls /FILE_COMMANDS_TEST_NON_SHELL_MODE", false, Command::CommandType::ls, "[]");

        // TODO find a scenario where the rm command fails

    /*
    current state of /FILE_COMMANDS_TEST:
    (empty)
    */

   // TODO find a way to test the upload command


    //         SerialManager::sharedInstance->commandLog("Uploading file to " + uploadPath.str() + " with size " + command.arguments[1] + " bytes");

    executeCommand("rm /FILE_COMMANDS_TEST/test.txt", true);
    executeCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/test.txt", true);
}