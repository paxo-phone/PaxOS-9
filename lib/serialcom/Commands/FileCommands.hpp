#pragma once

#include "../config.h"
#include "../CommandsManager.hpp"
#include "SerialManager.hpp"
#include <filestream.hpp>
#include <path.hpp>
#include <json.hpp>
#include <base64.hpp>
#include <string>
#include <delay.hpp>
using json = nlohmann::json;

// General use
#define PATH_DOES_NOT_EXIST(PATH) "Path " + std::string(PATH) + " does not exist."
#define PATH_ALREADY_EXISTS(PATH) "Path " + std::string(PATH) + " already exists."
#define DIRECTORY_DOES_NOT_EXIST(PATH) "Directory " + std::string(PATH) + " does not exist."
#define DIRECTORY_ALREADY_EXISTS(PATH) "Directory " + std::string(PATH) + " already exists."
#define FILE_DOES_NOT_EXIST(PATH) "File " + std::string(PATH) + " already exists."
#define FILE_ALREADY_EXISTS(PATH) "File " + std::string(PATH) + " already exists."

#define COMMAND_NOT_IMPLEMENTED(NAME) "Command " + std::string(NAME) + " is not implemented." 

// LS
#define LS_SUCCESS_OUPUT_HEADER(PATH) "Files and directories in " + std::string(PATH) + ":"

// TOUCH
#define FILE_CREATION_SUCCESS(PATH) "File " + std::string(PATH) + " created."
#define FILE_CREATION_FAILED(PATH) "Error creating file " + std::string(PATH) + "."

// MKDIR
#define DIRECTORY_CREATION_SUCCESS(PATH) "Directory " + std::string(PATH) + " created."
#define DIRECTORY_CREATION_FAILED(PATH) "Error creating directory " + std::string(PATH) + "."

// RM
#define PATH_REMOVAL_SUCCESS(PATH) "File or directory " + std::string(PATH) + " removed."
#define PATH_REMOVAL_FAILED(PATH) "Error removing file or directory " + std::string(PATH) + "."

// CP
#define COPY_SUCCESS(ORIGIN, DESTINATION) "File or directory " + std::string(ORIGIN) + " copied to " + std::string(DESTINATION) + "."
#define COPY_FAILED(ORIGIN, DESTINATION) "Error copying file or directory " + std::string(ORIGIN) + " to " + std::string(DESTINATION) + "."

// MV
#define MOVE_SUCCESS(ORIGIN, DESTINATION) "File or directory " + std::string(ORIGIN) + " moved to " + std::string(DESTINATION) + "."
#define MOVE_FAILED(ORIGIN, DESTINATION) "Error moving file or directory " + std::string(ORIGIN) + " to " + std::string(DESTINATION) + "."

// DOWNLOAD
#define FILE_CANNOT_BE_OPENED(PATH) "File " + std::string(PATH) + " cannot be opened."
#define FILE_SIZE_MESSAGE(SIZE) "File size " + std::string(SIZE) + "\n"

namespace serialcom
{
    // arg1 = path to get children from
    void CommandsManager::processLSCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        storage::Path lsPath = storage::Path(firstArgument);

        std::vector<std::string> files = lsPath.listdir();

        if (this->shellMode) {
            if (lsPath.exists()) {
                SerialManager::sharedInstance->commandLog(LS_SUCCESS_OUPUT_HEADER(firstArgument));
                for (const auto& file : files) {
                    SerialManager::sharedInstance->commandLog("\n" + file);
                    if (storage::Path(firstArgument + "/" + file).isdir())
                    {
                        SerialManager::sharedInstance->commandLog("/");
                    }
                }
            } else {
                SerialManager::sharedInstance->commandLog(DIRECTORY_DOES_NOT_EXIST(firstArgument));
            }
        } else {
            if (lsPath.exists()) {
                json output;

                std::vector<std::string> realFiles;
                std::vector<std::string> directories;

                for (const auto& file : files) {
                    if (storage::Path(firstArgument + "/" + file).isdir())
                    {
                        directories.push_back(file);
                    } else {
                        realFiles.push_back(file);
                    }
                }

                output["files"] = realFiles;
                output["directories"] = directories;
                
                SerialManager::sharedInstance->commandLog(output.dump());
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        }
    }

    // arg1 = file to create
    void CommandsManager::processTouchCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        storage::Path touchPath = storage::Path(firstArgument);

        if (touchPath.exists()) {
            if (this->shellMode) {
                SerialManager::sharedInstance->commandLog(FILE_ALREADY_EXISTS(firstArgument));
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        } else {
            if(touchPath.newfile())
            {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(FILE_CREATION_SUCCESS(firstArgument));

                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                }
            } else {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(FILE_CREATION_FAILED(firstArgument));
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                }
            }
        }
    }

    // arg1 = directory to create
    void CommandsManager::processMKDIRCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        storage::Path mkdirPath = storage::Path(firstArgument);

        if (mkdirPath.exists()) {
            if (this->shellMode)
            {
                SerialManager::sharedInstance->commandLog(DIRECTORY_ALREADY_EXISTS(firstArgument));
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        } else {
            if (mkdirPath.newdir())
            {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(DIRECTORY_CREATION_SUCCESS(firstArgument));
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                }
            } else {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(DIRECTORY_CREATION_FAILED(firstArgument));
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                }
            }
        }
    }

    // arg1 = file or directory to remove
    void CommandsManager::processRMCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        storage::Path rmPath = storage::Path(firstArgument);

        if (rmPath.exists()) {
            if (rmPath.remove())
            {
                if (this->shellMode) {
                    SerialManager::sharedInstance->commandLog(PATH_REMOVAL_SUCCESS(firstArgument));
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                }
            } else {
                if (this->shellMode) {
                    SerialManager::sharedInstance->commandLog(PATH_REMOVAL_FAILED(firstArgument));
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                }
            }
        } else {
            if (this->shellMode) {
                SerialManager::sharedInstance->commandLog(PATH_DOES_NOT_EXIST(firstArgument));
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        }
    }

    // arg1 = source file or directory
    // arg2 = destination file or directory
    // NOTE: the destination file or directory must not exist
    void CommandsManager::processCPCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];
        std::string secondArgument = command.arguments[1];

        storage::Path cpSourcePath = storage::Path(firstArgument);
        storage::Path cpDestinationPath = storage::Path(secondArgument);

        if (cpSourcePath.exists())
        {
            if (cpDestinationPath.exists())
            {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(PATH_ALREADY_EXISTS(secondArgument));
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                }
            } else {
                if (cpDestinationPath.copyTo(cpDestinationPath))
                {
                    if (this->shellMode)
                    {
                        SerialManager::sharedInstance->commandLog(COPY_SUCCESS(firstArgument, secondArgument));
                    } else {
                        SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);                    
                    }
                } else {
                    if (this->shellMode)
                    {
                        SerialManager::sharedInstance->commandLog(COPY_FAILED(firstArgument, secondArgument));
                    } else {
                        SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                    }
                }
            }
        } else {
            if (this->shellMode) {
                SerialManager::sharedInstance->commandLog(PATH_DOES_NOT_EXIST(firstArgument));
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        }
        
    }

    // arg1 = source file or directory
    // arg2 = destination file or directory
    // NOTE: the destination file or directory must not exist
    void CommandsManager::processMVCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];
        std::string secondArgument = command.arguments[1];

        storage::Path mvSourcePath = storage::Path(firstArgument);
        storage::Path mvDestinationPath = storage::Path(secondArgument);

        if (mvSourcePath.exists())
        {
            if (mvDestinationPath.exists())
            {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(PATH_ALREADY_EXISTS(secondArgument));
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                }
            } else {
                if (mvSourcePath.rename(mvDestinationPath))
                {
                    if (this->shellMode)
                    {
                        SerialManager::sharedInstance->commandLog(MOVE_SUCCESS(firstArgument, secondArgument));
                    } else {
                        SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                    }
                } else {
                    if (this->shellMode)
                    {
                        SerialManager::sharedInstance->commandLog(MOVE_FAILED(firstArgument, secondArgument));
                    } else {
                        SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                    }
                }
            }
        } else {
            if (this->shellMode) {
                SerialManager::sharedInstance->commandLog(PATH_DOES_NOT_EXIST(firstArgument));
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        }
    }

    // arg1 = file to read
    void CommandsManager::processCATCommand(const Command& command) const
    {
        // not implemented for the moment

        if (this->shellMode)
        {
            SerialManager::sharedInstance->commandLog(COMMAND_NOT_IMPLEMENTED(std::string("cat")));
        } else {
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
    }

    // arg1 = file to download
    // output = file content in chunks of 2048 bytes (or less for the last chunk), each one encoded in base64
    void CommandsManager::processDownloadCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        storage::Path downloadPath = storage::Path(firstArgument);

        if (!downloadPath.exists())
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(PATH_DOES_NOT_EXIST(firstArgument));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            return;
        }

        storage::FileStream fileStream(downloadPath.str(), storage::READ);

        if (!fileStream.isopen())
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(FILE_CANNOT_BE_OPENED(firstArgument));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            return;
        }
        #define BOOL_STR(b) (b?"true":"false")

        if (this->shellMode)
            //SerialManager::sharedInstance->commandLog("File size " + std::to_string(fileStream.size()) + " : " + std::to_string(fileStream.size() + 1) + " : " + BOOL_STR(fileStream.size() == 0));
            SerialManager::sharedInstance->commandLog(FILE_SIZE_MESSAGE(std::to_string(fileStream.size())));
        else
            SerialManager::sharedInstance->commandLog(std::to_string(fileStream.size()));

        if (fileStream.size() == 0)
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog("Nothing in the file");
            return;
        }

        if (!this->shellMode)
            SerialManager::sharedInstance->commandLog("\n");

        char nextChar = fileStream.readchar();
        while (nextChar != std::char_traits<char>::eof() && fileStream.sizeFromCurrentPosition() > 0)
        {
            //SerialManager::sharedInstance->commandLog(std::to_string((uint8_t)nextChar));
            //SerialManager::sharedInstance->commandLog(std::to_string(fileStream.sizeFromCurrentPosition()));
            std::string chunk = nextChar + fileStream.read(2047);
            std::string encodedChunk = base64::to_base64(chunk);
            SerialManager::sharedInstance->commandLog(encodedChunk);
            
            nextChar = fileStream.readchar();
        }
    
        fileStream.close();
    }

    // arg1 = fileName to upload
    // arg2 = size of the file in bytes
    // input = file content in chunks of 2048 bytes (or less for the last chunk), each one encoded in base64
    void CommandsManager::processUploadCommand(const Command& command) const
    {
        storage::Path uploadPath(command.arguments[0]);
        SerialManager::sharedInstance->commandLog("Uploading file to " + uploadPath.str() + " with size " + command.arguments[1] + " bytes");
        try
        {
            size_t fileSize = std::stoi(command.arguments[1]);
            uploadPath.newfile();

            storage::FileStream fileStream(uploadPath.str(), storage::WRITE);

            for (size_t i = 0; i < fileSize; i += 2048)
            {
                //SerialManager::sharedInstance->commandLog("Getting encoded chunk");
                std::string encodedChunk;
                while (encodedChunk.empty())
                {
                    static size_t ndx = 0;
                    bool newData = false;
                    char endMarker = '\n';
                    char rc;
                    
                    #ifdef ESP_PLATFORM
                    #define dataAvailable Serial.available() > 0
                    #else
                    #define dataAvailable std::cin.peek() != EOF
                    #endif
                    while (dataAvailable && newData == false) {
                        #ifdef ESP_PLATFORM
                        rc = Serial.read();
                        #else
                        rc = std::cin.get();
                        #endif

                        if (rc != endMarker)
                            encodedChunk += rc;     
                        else
                            newData = true;
                    }
                }

                encodedChunk.pop_back(); // remove the last '\n' character
                /*
                while (true)
                {
                    SerialManager::sharedInstance->commandLog("Waiting for chunk");
                    std::getline(std::cin, encodedChunk);
                    if (encodedChunk.size() != 0)
                    {
                        SerialManager::sharedInstance->commandLog("Got a chunk whose size is not null.");
                        break;
                    }

                    PaxOS_Delay(100);
                }
                */
                //SerialManager::sharedInstance->commandLog("Got chunk");
                //SerialManager::sharedInstance->commandLog(std::to_string(encodedChunk.size()));
                //SerialManager::sharedInstance->commandLog(encodedChunk);
                std::string chunk = base64::from_base64(encodedChunk).c_str();
                //SerialManager::sharedInstance->commandLog(chunk);
                fileStream.write(chunk);
            }

            fileStream.close();

            if (this->shellMode)
                SerialManager::sharedInstance->commandLog("File " + uploadPath.str() + " with size " + std::to_string(fileSize) + " uploaded successfully.");
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
        }
        catch(const std::exception& e)
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog("Error uploading file: " + std::string(e.what()));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            return;
        }
    }
} // namespace serialcom
