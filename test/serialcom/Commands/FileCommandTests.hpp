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

    testCommand(
        "mkdir /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::mkdir,
        DIRECTORY_CREATION_SUCCESS("/FILE_COMMANDS_TEST")
    );
    testCommand(
        "mkdir /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::mkdir,
        NON_SHELL_MODE_NO_ERROR_CODE
    );

    testCommand(
        "mkdir /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::mkdir,
        DIRECTORY_ALREADY_EXISTS("/FILE_COMMANDS_TEST")
    );
    testCommand(
        "mkdir /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::mkdir,
        NON_SHELL_MODE_ERROR_CODE
    );

    // testCommand("mkdir FILE_COMMANDS_TESTç*\"+_=?%:\n\t\r", true, Command::CommandType::mkdir,
    // "Error creating directory FILE_COMMANDS_TESTç*\"+_=?%:\n\t\r"); testCommand("mkdir
    // FILE_COMMANDS_TEST_NON_SHELL_MODEç*\"+_=?%:\n\t\r", false, Command::CommandType::mkdir,
    // NON_SHELL_MODE_ERROR_CODE);

    /*
    current state of /FILE_COMMANDS_TEST:
    (empty)
    */

    testCommand(
        "ls /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::ls,
        LS_SUCCESS_OUPUT_HEADER("/FILE_COMMANDS_TEST")
    );
    testCommand(
        "ls /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::ls,
        "{\"directories\":[],\"files\":[]}"
    );

    testCommand(
        "ls /FILE_COMMANDS_TEST1",
        true,
        Command::CommandType::ls,
        DIRECTORY_DOES_NOT_EXIST("/FILE_COMMANDS_TEST1")
    );
    testCommand(
        "ls /FILE_COMMANDS_TEST_NON_SHELL_MODE1",
        false,
        Command::CommandType::ls,
        NON_SHELL_MODE_ERROR_CODE
    );

    executeCommand("rm /FILE_COMMANDS_TEST/file.txt", true);
    executeCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt", true);

    testCommand(
        "touch /FILE_COMMANDS_TEST/file.txt",
        true,
        Command::CommandType::touch,
        FILE_CREATION_SUCCESS("/FILE_COMMANDS_TEST/file.txt")
    );
    testCommand(
        "touch /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt",
        false,
        Command::CommandType::touch,
        NON_SHELL_MODE_NO_ERROR_CODE
    );

    testCommand(
        "touch /FILE_COMMANDS_TEST/file.txt",
        true,
        Command::CommandType::touch,
        FILE_ALREADY_EXISTS("/FILE_COMMANDS_TEST/file.txt")
    );
    testCommand(
        "touch /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt",
        false,
        Command::CommandType::touch,
        NON_SHELL_MODE_ERROR_CODE
    );

    // testCommand("touch FILE_COMMANDS_TESTç*\"+_=?%:\n\t\r/file.txt", true,
    // Command::CommandType::touch, "Error creating file
    // /FILE_COMMANDS_TESTç*\"+_=?%:\n\t\r/file.txt"); testCommand("touch
    // FILE_COMMANDS_TEST_NON_SHELL_MODEç*\"+_=?%:\n\t\r/file.txt", false,
    // Command::CommandType::touch, NON_SHELL_MODE_ERROR_CODE);

    /*
    current state of /FILE_COMMANDS_TEST:
    file.txt
    */

    testCommand(
        "ls /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::ls,
        LS_SUCCESS_OUPUT_HEADER("/FILE_COMMANDS_TEST") + "\nfile.txt"
    );
    testCommand(
        "ls /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::ls,
        "{\"directories\":[],\"files\":[\"file.txt\"]}"
    );

    testCommand(
        "rm /FILE_COMMANDS_TEST/file.txt",
        true,
        Command::CommandType::rm,
        PATH_REMOVAL_SUCCESS("/FILE_COMMANDS_TEST/file.txt")
    );
    testCommand(
        "rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt",
        false,
        Command::CommandType::rm,
        NON_SHELL_MODE_NO_ERROR_CODE
    );

    testCommand(
        "rm /FILE_COMMANDS_TEST/file.txt",
        true,
        Command::CommandType::rm,
        PATH_DOES_NOT_EXIST("/FILE_COMMANDS_TEST/file.txt")
    );
    testCommand(
        "rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/file.txt",
        false,
        Command::CommandType::rm,
        NON_SHELL_MODE_ERROR_CODE
    );

    testCommand(
        "ls /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::ls,
        LS_SUCCESS_OUPUT_HEADER("/FILE_COMMANDS_TEST")
    );
    testCommand(
        "ls /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::ls,
        "{\"directories\":[],\"files\":[]}"
    );

    // TODO find a scenario where the rm command fails

    /*
    current state of /FILE_COMMANDS_TEST:
    (empty)
    */

    // TODO find a way to test the upload command

    //         SerialManager::sharedInstance->commandLog("Uploading file to " + uploadPath.str() + "
    //         with size " + command.arguments[1] + " bytes");

    // executeCommand("rm /FILE_COMMANDS_TEST/test.txt", true);
    // executeCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/test.txt", true);

    // testCommand("upload /FILE_COMMANDS_TEST/test.txt 10\0MTIzNDU2Nzg5MA==", true,
    // Command::CommandType::upload, "Uploading file to /FILE_COMMANDS_TEST/test.txt with size 10
    // bytes\nFile /FILE_COMMANDS_TEST/test.txt with size 10 uploaded successfully.\n");
    // testCommand("upload /FILE_COMMANDS_TEST_NON_SHELL_MODE/test.txt 10\0MTIzNDU2Nzg5MA==", false,
    // Command::CommandType::upload, NON_SHELL_MODE_NO_ERROR_CODE);

    // testCommand("download /FILE_COMMANDS_TEST/test.txt", true, Command::CommandType::download,
    // "File size 10\nMTIzNDU2Nzg5MA==\nNo error.\n"); testCommand("download
    // /FILE_COMMANDS_TEST_NON_SHELL_MODE/test.txt", false, Command::CommandType::download,
    // "10\nMTIzNDU2Nzg5MA==");

    testCommand(
        "download /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::download,
        FILE_CANNOT_BE_OPENED("/FILE_COMMANDS_TEST")
    );
    testCommand(
        "download /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::download,
        NON_SHELL_MODE_ERROR_CODE
    );

    executeCommand("rm /FILE_COMMANDS_TEST/emptyFile.txt", true);
    executeCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFile.txt", true);

    testCommand(
        "touch /FILE_COMMANDS_TEST/emptyFile.txt",
        true,
        Command::CommandType::touch,
        FILE_CREATION_SUCCESS("/FILE_COMMANDS_TEST/emptyFile.txt")
    );
    testCommand(
        "touch /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFile.txt",
        false,
        Command::CommandType::touch,
        NON_SHELL_MODE_NO_ERROR_CODE
    );

    testCommand(
        "ls /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::ls,
        LS_SUCCESS_OUPUT_HEADER("/FILE_COMMANDS_TEST") + "\nemptyFile.txt"
    );
    testCommand(
        "ls /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::ls,
        "{\"directories\":[],\"files\":[\"emptyFile.txt\"]}"
    );

    // testCommand("download /FILE_COMMANDS_TEST/emptyFile.txt", true,
    // Command::CommandType::download, "File size 0\nNothing in the file"); testCommand("download
    // /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFile.txt", false, Command::CommandType::download,
    // "0");

    // cp and mv tests, read result to make sure it works

    executeCommand("rm /FILE_COMMANDS_TEST/emptyFileCopy.txt", true);
    executeCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFileCopy.txt", true);

    testCommand(
        "cp /FILE_COMMANDS_TEST/emptyFile.txt /FILE_COMMANDS_TEST/emptyFileCopy.txt",
        true,
        Command::CommandType::cp,
        COPY_SUCCESS("/FILE_COMMANDS_TEST/emptyFile.txt", "/FILE_COMMANDS_TEST/emptyFileCopy.txt")
    );
    testCommand(
        "cp /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFile.txt "
        "/FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFileCopy.txt",
        false,
        Command::CommandType::cp,
        NON_SHELL_MODE_NO_ERROR_CODE
    );

    // testCommand("cp FILE_COMMANDS_TEST/test.txt FILE_COMMANDS_TEST/testCopy.txt", true,
    // Command::CommandType::cp, "File FILE_COMMANDS_TEST/test.txt copied to
    // FILE_COMMANDS_TEST/testCopy.txt.\n"); testCommand("cp
    // FILE_COMMANDS_TEST_NON_SHELL_MODE/test.txt FILE_COMMANDS_TEST_NON_SHELL_MODE/testCopy.txt",
    // false, Command::CommandType::cp, NON_SHELL_MODE_NO_ERROR_CODE);

    testCommand(
        "ls /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::ls,
        LS_SUCCESS_OUPUT_HEADER("/FILE_COMMANDS_TEST") + "\nemptyFile.txt\nemptyFileCopy.txt"
    );
    testCommand(
        "ls /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::ls,
        "{\"directories\":[],\"files\":[\"emptyFile.txt\",\"emptyFileCopy.txt\"]}"
    );

    executeCommand("rm /FILE_COMMANDS_TEST/emptyFileMoved.txt", true);
    executeCommand("rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFileMoved.txt", true);

    testCommand(
        "mv /FILE_COMMANDS_TEST/emptyFile.txt /FILE_COMMANDS_TEST/emptyFileMoved.txt",
        true,
        Command::CommandType::mv,
        MOVE_SUCCESS("/FILE_COMMANDS_TEST/emptyFile.txt", "/FILE_COMMANDS_TEST/emptyFileMoved.txt")
    );
    testCommand(
        "mv /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFile.txt "
        "/FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFileMoved.txt",
        false,
        Command::CommandType::mv,
        NON_SHELL_MODE_NO_ERROR_CODE
    );

    // testCommand("mv FILE_COMMANDS_TEST/test.txt FILE_COMMANDS_TEST/testMoved.txt", true,
    // Command::CommandType::mv, "File FILE_COMMANDS_TEST/test.txt moved to
    // FILE_COMMANDS_TEST/testMoved.txt.\n"); testCommand("mv
    // FILE_COMMANDS_TEST_NON_SHELL_MODE/test.txt FILE_COMMANDS_TEST_NON_SHELL_MODE/testMoved.txt",
    // false, Command::CommandType::mv, NON_SHELL_MODE_NO_ERROR_CODE);

    testCommand(
        "ls /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::ls,
        LS_SUCCESS_OUPUT_HEADER("/FILE_COMMANDS_TEST") + "\nemptyFileCopy.txt\nemptyFileMoved.txt"
    );
    testCommand(
        "ls /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::ls,
        "{\"directories\":[],\"files\":[\"emptyFileCopy.txt\",\"emptyFileMoved.txt\"]}"
    );

    // testCommand("download /FILE_COMMANDS_TEST/emptyFileMoved.txt", true,
    // Command::CommandType::download, "File size 0\nNothing in the file"); testCommand("download
    // /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFileMoved.txt", false,
    // Command::CommandType::download, "0");

    // testCommand("download FILE_COMMANDS_TEST/testMoved.txt", true,
    // Command::CommandType::download, "File size 10\nMTIzNDU2Nzg5MA==\nNo error.\n");
    // testCommand("download FILE_COMMANDS_TEST_NON_SHELL_MODE/testMoved.txt", false,
    // Command::CommandType::download, "10\nMTIzNDU2Nzg5MA==");

    // testCommand("download /FILE_COMMANDS_TEST/emptyFileCopy.txt", true,
    // Command::CommandType::download, "File size 0\nNothing in the file"); testCommand("download
    // /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFileCopy.txt", false, Command::CommandType::download,
    // "0");

    // testCommand("download FILE_COMMANDS_TEST/testCopy.txt", true, Command::CommandType::download,
    // "File size 10\nMTIzNDU2Nzg5MA==\nNo error.\n"); testCommand("download
    // FILE_COMMANDS_TEST_NON_SHELL_MODE/testCopy.txt", false, Command::CommandType::download,
    // "10\nMTIzNDU2Nzg5MA==");

    // rm tests

    testCommand(
        "rm /FILE_COMMANDS_TEST/emptyFileMoved.txt",
        true,
        Command::CommandType::rm,
        PATH_REMOVAL_SUCCESS("/FILE_COMMANDS_TEST/emptyFileMoved.txt")
    );
    testCommand(
        "rm /FILE_COMMANDS_TEST_NON_SHELL_MODE/emptyFileMoved.txt",
        false,
        Command::CommandType::rm,
        NON_SHELL_MODE_NO_ERROR_CODE
    );

    // testCommand("rm FILE_COMMANDS_TEST/testMoved.txt", true, Command::CommandType::rm, "File or
    // directory FILE_COMMANDS_TEST/testMoved.txt removed.\n"); testCommand("rm
    // FILE_COMMANDS_TEST_NON_SHELL_MODE/testMoved.txt", false, Command::CommandType::rm,
    // NON_SHELL_MODE_NO_ERROR_CODE);

    testCommand(
        "rm /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::rm,
        PATH_REMOVAL_SUCCESS("/FILE_COMMANDS_TEST")
    );
    testCommand(
        "rm /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::rm,
        NON_SHELL_MODE_NO_ERROR_CODE
    );

    testCommand(
        "ls /FILE_COMMANDS_TEST",
        true,
        Command::CommandType::ls,
        DIRECTORY_DOES_NOT_EXIST("/FILE_COMMANDS_TEST")
    );
    testCommand(
        "ls /FILE_COMMANDS_TEST_NON_SHELL_MODE",
        false,
        Command::CommandType::ls,
        NON_SHELL_MODE_ERROR_CODE
    );
}
