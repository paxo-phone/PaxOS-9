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
    /* if !shellMode
        1. OK + 4 bytes (file size)
        2. file content in chunks of 2048 bytes (or less for the last chunk)
        3. OK
    */
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
        else {
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);

            uint32_t fileSize = fileStream.size();

            std::string fileSizeString(reinterpret_cast<const char *>(&fileSize), sizeof(fileSize));
            SerialManager::sharedInstance->commandLog(fileSizeString);
            SerialManager::sharedInstance->finishCommandLog(false);
        }

        if (fileStream.size() == 0 && this->shellMode)
        {
            SerialManager::sharedInstance->commandLog("Nothing in the file");
            fileStream.close();
            return;
        }

        std::string chunk;

        char nextChar = fileStream.readchar();
        while (nextChar != std::char_traits<char>::eof() && fileStream.sizeFromCurrentPosition() > 0)
        {
            chunk += nextChar;

            chunk += fileStream.read(2047);

            if (this->shellMode)
            {
                std::string encodedChunk = base64::to_base64(chunk);
                SerialManager::sharedInstance->singleCommandLog(encodedChunk);
            }
            else
            {
                SerialManager::sharedInstance->singleCommandLog(chunk);
            }
            chunk.clear();
            
            nextChar = fileStream.readchar();
        }
        
        fileStream.close();

        if (this->shellMode)
        {
            SerialManager::sharedInstance->startCommandLog();
            SerialManager::sharedInstance->commandLog("No error.");
        }
        else
        {
            SerialManager::sharedInstance->startCommandLog();
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
        }
    }

    // arg1 = fileName to upload
    // arg2 = size of the file in bytes
    // input = file content in chunks of 2048 bytes (or less for the last chunk), each one encoded in base64
    void CommandsManager::processUploadCommand(const Command& command) const
    {
        storage::Path uploadPath(command.arguments[0]);
        //SerialManager::sharedInstance->commandLog("Uploading file to " + uploadPath.str() + " with size " + command.arguments[1] + " bytes");
        try
        {
            size_t fileSize = std::stoi(command.arguments[1]);
            uploadPath.newfile();

            storage::FileStream fileStream(uploadPath.str(), storage::WRITE);

            if (this->shellMode)
            {
                for (size_t i = 0; i < fileSize; i += 2048)
                {
                    //SerialManager::sharedInstance->commandLog("Getting encoded chunk");
                    std::string encodedChunk;
                    while (encodedChunk.empty())
                    {
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
                    std::string chunk = base64::from_base64(encodedChunk).c_str();
                    fileStream.write(chunk);
                }
            } else
            {

                size_t receivedSize = 0;

                SerialManager::sharedInstance->singleCommandLog(NON_SHELL_MODE_NO_ERROR_CODE);

                while (receivedSize < fileSize)
                {
                    //SerialManager::sharedInstance->singleCommandLog("main loop");
                    std::string tempBuffer;
                    uint16_t expectedSize = 0;

                    int tries = 0;
                    uint16_t tempBufferSize = 0;
                    bool newData = false;

                    while (true)
                    {
                        //SerialManager::sharedInstance->singleCommandLog("sec loop");
                        if (tries >= 100 && false)
                        {
                            fileStream.close();
                            uploadPath.remove();
                            SerialManager::sharedInstance->singleCommandLog("Input found: " + tempBuffer);
                            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                            return;
                        }
                        tries++;
                        
                        #ifdef ESP_PLATFORM
                        #define dataAvailable Serial.available() > 0
                        #else
                        #define dataAvailable std::cin.peek() != EOF
                        #endif
                        while (dataAvailable && newData == false && tempBufferSize < 2060) {
                            //SerialManager::sharedInstance->singleCommandLog("in loop");
                            #ifdef ESP_PLATFORM
                            char rc = Serial.read();
                            #else
                            char rc = std::cin.get();
                            #endif

                            if (rc != -1)
                            {
                                tempBuffer += rc;
                                tempBufferSize++;   
                            }  
                            else
                                newData = true;
                        }

                        size_t pos = tempBuffer.find(std::string((char)0xff + "") + (char)0xfe + (char)0xfd);

                        if (pos == std::string::npos)
                        {
                            tempBuffer = tempBuffer.substr(max((uint16_t)2, tempBufferSize) - 2); // keep the last 2 bytes that could be the beginning of the next message
                            tempBufferSize = min(tempBufferSize, (uint16_t)2);
                            continue;
                        }

                        tempBuffer = tempBuffer.substr(pos + 2);

                        tempBufferSize -= pos + 2;

                        if (tempBufferSize > 8)
                        {
                            //SerialManager::sharedInstance->commandLog("GOT MORE THAN 8 BYTES!");
                            expectedSize = tempBuffer[0] + (tempBuffer[1] << 8);
                            //SerialManager::sharedInstance->commandLog("First size byte is " + std::to_string(tempBuffer[0]) + " second size byte is " + std::to_string(tempBuffer[1]));
                            //SerialManager::sharedInstance->commandLog("Size is " + std::to_string(expectedSize) + " buffersize = " + std::to_string(tempBufferSize));
                            //SerialManager::sharedInstance->commandLog("Buffer itself: " + tempBuffer);
                            //SerialManager::sharedInstance->finishCommandLog(false);
                            
                            if (tempBufferSize >= expectedSize + 8)
                                break;
                        }

                        PaxOS_Delay(20);
                    }

                    uint16_t options = tempBuffer[2] + (tempBuffer[3] << 8);

                    uint32_t checksum = tempBuffer[4] + (tempBuffer[5] << 8) + (tempBuffer[6] << 16) + (tempBuffer[7] << 24);

                    uint64_t tempChecksum = 0;

                    tempBuffer = tempBuffer.substr(8, expectedSize);
                    
                    for (char c : tempBuffer)
                    {
                        tempChecksum = (tempChecksum + c) % 4294967295;
                    }

                    if (checksum != (uint32_t)tempChecksum)
                    {
                        fileStream.close();
                        uploadPath.remove();
                        SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE + std::string("checksum don't match, expected ") + std::to_string(checksum) + " got " + std::to_string(tempChecksum));
                        return;
                    }

                    fileStream.write(tempBuffer);

                    receivedSize += expectedSize;

                    tempBuffer = "";

                    SerialManager::sharedInstance->singleCommandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                }
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
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE + std::string(e.what()));

            uploadPath.remove();
            return;
        }
    }
} // namespace serialcom
