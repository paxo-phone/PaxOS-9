#pragma once

#include "../CommandsManager.hpp"
#include "../config.h"
#include "SerialManager.hpp"

#include <base64.hpp>
#include <delay.hpp>
#include <filestream.hpp>
#include <json.hpp>
#include <path.hpp>
#include <string>
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
#define COPY_SUCCESS(ORIGIN, DESTINATION)                                                          \
    "File or directory " + std::string(ORIGIN) + " copied to " + std::string(DESTINATION) + "."
#define COPY_FAILED(ORIGIN, DESTINATION)                                                           \
    "Error copying file or directory " + std::string(ORIGIN) + " to " + std::string(DESTINATION) + \
        "."

// MV
#define MOVE_SUCCESS(ORIGIN, DESTINATION)                                                          \
    "File or directory " + std::string(ORIGIN) + " moved to " + std::string(DESTINATION) + "."
#define MOVE_FAILED(ORIGIN, DESTINATION)                                                           \
    "Error moving file or directory " + std::string(ORIGIN) + " to " + std::string(DESTINATION) +  \
        "."

// DOWNLOAD
#define FILE_CANNOT_BE_OPENED(PATH) "File " + std::string(PATH) + " cannot be opened."
#define FILE_SIZE_MESSAGE(SIZE) "File size " + std::string(SIZE) + "\n"

// UPLOAD
#define NON_SHELL_CHUNK_NEED_TRANSFER_AGAIN "RE"

namespace serialcom
{
    // arg1 = path to get children from
    void CommandsManager::processLSCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        storage::Path lsPath = storage::Path(firstArgument);

        std::vector<std::string> files = lsPath.listdir();

        if (this->shellMode)
        {
            if (lsPath.exists())
            {
                SerialManager::sharedInstance->commandLog(LS_SUCCESS_OUPUT_HEADER(firstArgument));
                for (const auto& file : files)
                {
                    SerialManager::sharedInstance->commandLog("\n" + file);
                    if (storage::Path(firstArgument + "/" + file).isdir())
                        SerialManager::sharedInstance->commandLog("/");
                }
            }
            else
            {
                SerialManager::sharedInstance->commandLog(DIRECTORY_DOES_NOT_EXIST(firstArgument));
            }
        }
        else if (lsPath.exists())
        {
            json output;

            std::vector<std::string> realFiles;
            std::vector<std::string> directories;

            for (const auto& file : files)
                if (storage::Path(firstArgument + "/" + file).isdir())
                    directories.push_back(file);
                else
                    realFiles.push_back(file);

            output["files"] = realFiles;
            output["directories"] = directories;

            SerialManager::sharedInstance->commandLog(output.dump());
        }
        else
        {
            SerialManager::sharedInstance->commandLog(
                NON_SHELL_MODE_ERROR_CODE + std::string("Directory ") + firstArgument +
                " does not exist."
            );
        }
    }

    // arg1 = file to create
    void CommandsManager::processTouchCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        storage::Path touchPath = storage::Path(firstArgument);

        if (touchPath.exists())
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(FILE_ALREADY_EXISTS(firstArgument));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
        else
        {
            if (touchPath.newfile())
                if (this->shellMode)
                    SerialManager::sharedInstance->commandLog(FILE_CREATION_SUCCESS(firstArgument));

                else
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
            else if (this->shellMode)
                SerialManager::sharedInstance->commandLog(FILE_CREATION_FAILED(firstArgument));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
    }

    // arg1 = directory to create
    void CommandsManager::processMKDIRCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        storage::Path mkdirPath = storage::Path(firstArgument);

        if (mkdirPath.exists())
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(DIRECTORY_ALREADY_EXISTS(firstArgument));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
        else if (mkdirPath.newdir())
        {
            if (this->shellMode)
            {
                SerialManager::sharedInstance->commandLog(
                    DIRECTORY_CREATION_SUCCESS(firstArgument)
                );
            }
            else
            {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
            }
        }
        else if (this->shellMode)
        {
            SerialManager::sharedInstance->commandLog(DIRECTORY_CREATION_FAILED(firstArgument));
        }
        else
        {
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
    }

    // arg1 = file or directory to remove
    void CommandsManager::processRMCommand(const Command& command) const
    {
        std::string firstArgument = command.arguments[0];

        storage::Path rmPath = storage::Path(firstArgument);

        if (rmPath.exists())
        {
            if (rmPath.remove())
                if (this->shellMode)
                    SerialManager::sharedInstance->commandLog(PATH_REMOVAL_SUCCESS(firstArgument));
                else
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
            else if (this->shellMode)
                SerialManager::sharedInstance->commandLog(PATH_REMOVAL_FAILED(firstArgument));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
        else
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(PATH_DOES_NOT_EXIST(firstArgument));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
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
                    SerialManager::sharedInstance->commandLog(PATH_ALREADY_EXISTS(secondArgument));
                else
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
            else if (cpDestinationPath.copyTo(cpDestinationPath))
            {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(
                        COPY_SUCCESS(firstArgument, secondArgument)
                    );
                }
                else
                {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                }
            }
            else if (this->shellMode)
            {
                SerialManager::sharedInstance->commandLog(
                    COPY_FAILED(firstArgument, secondArgument)
                );
            }
            else
            {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        }
        else
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(PATH_DOES_NOT_EXIST(firstArgument));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
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
                    SerialManager::sharedInstance->commandLog(PATH_ALREADY_EXISTS(secondArgument));
                else
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
            else if (mvSourcePath.rename(mvDestinationPath))
            {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(
                        MOVE_SUCCESS(firstArgument, secondArgument)
                    );
                }
                else
                {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                }
            }
            else if (this->shellMode)
            {
                SerialManager::sharedInstance->commandLog(
                    MOVE_FAILED(firstArgument, secondArgument)
                );
            }
            else
            {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        }
        else
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(PATH_DOES_NOT_EXIST(firstArgument));
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
    }

    // arg1 = file to read
    void CommandsManager::processCATCommand(const Command& command) const
    {
        // not implemented for the moment

        if (this->shellMode)
            SerialManager::sharedInstance->commandLog(COMMAND_NOT_IMPLEMENTED(std::string("cat")));
        else
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
    }

    // WARNING: you have to set the command id of a download command to something with trailing
    // zeros, the index of the chunk sent will be added (+) to the two last bytey of the command id.
    // first chunk index is 1, max index is 2^16 - 1 = 65535
    // arg1 = file to download
    // output = file content in chunks of 2048 bytes (or less for the last chunk), each one encoded
    // in base64
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
#define BOOL_STR(b) (b ? "true" : "false")

        if (this->shellMode)
            // SerialManager::sharedInstance->commandLog("File size " +
            // std::to_string(fileStream.size()) + " : " + std::to_string(fileStream.size() + 1) + "
            // : " + BOOL_STR(fileStream.size() == 0));
            SerialManager::sharedInstance->commandLog(
                FILE_SIZE_MESSAGE(std::to_string(fileStream.size()))
            );
        else
        {
            uint32_t fileSize = fileStream.size();

            std::string fileSizeString(reinterpret_cast<const char*>(&fileSize), sizeof(fileSize));
            SerialManager::sharedInstance->singleCommandLog(
                NON_SHELL_MODE_NO_ERROR_CODE + fileSizeString,
                command.command_id
            );
        }

        if (fileStream.size() == 0 && this->shellMode)
        {
            SerialManager::sharedInstance->commandLog("Nothing in the file");
            fileStream.close();
            return;
        }

        uint16_t chunkIndex = 1;

#define CHUNK_SIZE 2048

        while (true)
        {
            char buffer[CHUNK_SIZE];

            std::streamsize readSize = fileStream.read(buffer, CHUNK_SIZE);

            if (readSize <= 0)
            {
                break;
            } // EOF or no more data

            std::string chunk(buffer, static_cast<size_t>(readSize));

            if (this->shellMode)
            {
                std::string encodedChunk = base64::to_base64(chunk);
                SerialManager::sharedInstance->singleCommandLog(encodedChunk);
            }
            else
            {
                // add chunkIndex to the command id (ex command id is 0x1234567890123400, chunkIndex
                // is 1, command id will be 0x1234567890123401)
                char commandIdWithChunkIndex[COMMAND_ID_SIZE];
                memcpy(commandIdWithChunkIndex, command.command_id, COMMAND_ID_SIZE);
                uint16_t truncatedIndex = static_cast<uint16_t>(chunkIndex & 0xFFFF);
                commandIdWithChunkIndex[COMMAND_ID_SIZE - 1] =
                    static_cast<char>(truncatedIndex & 0xFF);
                commandIdWithChunkIndex[COMMAND_ID_SIZE - 2] =
                    static_cast<char>((truncatedIndex >> 8) & 0xFF);

                chunkIndex++;

                SerialManager::sharedInstance->singleCommandLog(chunk, commandIdWithChunkIndex);
            }
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
    // input = file content in chunks of 2048 bytes (or less for the last chunk), each one encoded
    // in base64
    /* NON SHELL MODE
        -> upload filename size
        <- OK
        -> chunk (undetermined size, little is more reliable but slower)
        {
            <- RE (if the chunk is not correct) =>
                {
                    -> send chunk again
                    OR
                    -> send a packet with the first bit of the options being set to 1 => end the
       communication
                }
            OR
            <- OK => send next chunk
            OR
            <- KO => end of the communication because of an error
        }
        <- OK
    */
    void CommandsManager::processUploadCommand(const Command& command) const
    {
        storage::Path uploadPath(command.arguments[0]);
        // SerialManager::sharedInstance->commandLog("Uploading file to " + uploadPath.str() + "
        // with size " + command.arguments[1] + " bytes");
        try
        {
            size_t fileSize = std::stoi(command.arguments[1]);
            uploadPath.newfile();

            storage::FileStream fileStream(uploadPath.str(), storage::WRITE);

            if (this->shellMode)
            {
                for (size_t i = 0; i < fileSize; i += 2048)
                {
                    // SerialManager::sharedInstance->commandLog("Getting encoded chunk");
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
                        while (dataAvailable && newData == false)
                        {
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
            }
            else
            {
                size_t receivedSize = 0;
                SerialManager::sharedInstance->singleCommandLog(
                    NON_SHELL_MODE_NO_ERROR_CODE,
                    command.command_id
                );

                uint16_t askAgainTries = 0;

                std::string tempBuffer;
                tempBuffer.reserve(
                    4096
                ); // Pre-reserve memory to avoid reallocations while reading.

                while (receivedSize < fileSize)
                {
                    if (askAgainTries >= 10)
                    {
                        SerialManager::sharedInstance->singleCommandLog(
                            NON_SHELL_MODE_ERROR_CODE + std::string("Too many tries, aborting."),
                            command.command_id
                        );
                        uploadPath.remove();
                        fileStream.close();
                        return;
                    }

                    std::string_view packetView;

                    uint16_t timeoutTries = 0;
                    bool shouldAbortPacket = false;

                    uint16_t expectedDataSize = 0;

                    while (true)
                    {
                        PaxOS_Delay(5);

                        bool newDataRead = false;
#ifdef ESP_PLATFORM
#define dataAvailable (Serial.available() > 0)
#else
#define dataAvailable std::cin.peek() != EOF
#endif
                        while (dataAvailable && tempBuffer.size() < 4096)
                        {
#ifdef ESP_PLATFORM
                            int rc = Serial.read();
#else
                            int rc = std::cin.get();
#endif
                            if (rc != -1)
                            {
                                tempBuffer += static_cast<char>(rc);
                                newDataRead = true;
                            }
                        }

                        if (newDataRead)
                        {
                            timeoutTries = 0;
                        }
                        else
                        {
                            timeoutTries++;
                            if (timeoutTries >= 1000)
                            { // 5ms * 1000 = 5 second timeout
                                shouldAbortPacket = true;
                                break;
                            }
                        }

                        std::string_view bufferView(tempBuffer);
                        constexpr std::string_view MAGIC_BYTES("\xFF\xFE\xFD", 3);

                        size_t magicPos = bufferView.find(MAGIC_BYTES);

                        if (magicPos == std::string_view::npos)
                        {
                            if (bufferView.size() >= MAGIC_BYTES.size())
                                tempBuffer.erase(0, bufferView.size() - (MAGIC_BYTES.size() - 1));
                            continue;
                        }

                        if (magicPos > 0)
                        {
                            tempBuffer.erase(0, magicPos);
                            bufferView = tempBuffer;
                        }

                        // [Magic 3B][CmdID 8B][PayloadSize 2B] = 13B
                        constexpr size_t HEADER_PREFIX_LEN = 13;
                        if (bufferView.size() < HEADER_PREFIX_LEN)
                            continue;

                        // We have enough for the prefix, now read the expected data payload size.
                        expectedDataSize = static_cast<uint8_t>(bufferView[11]) +
                                           (static_cast<uint8_t>(bufferView[12]) << 8);

                        // [Prefix 13B][Options 2B][Checksum 4B][Index 2B][Data]
                        constexpr size_t METADATA_LEN = 8;
                        const size_t totalPacketSize =
                            HEADER_PREFIX_LEN + METADATA_LEN + expectedDataSize;

                        if (bufferView.size() < totalPacketSize)
                            continue;

                        packetView = bufferView.substr(0, totalPacketSize);
                        break;
                    }

                    if (shouldAbortPacket)
                    {
                        SerialManager::sharedInstance->singleCommandLog(
                            NON_SHELL_CHUNK_NEED_TRANSFER_AGAIN,
                            command.command_id
                        );
                        askAgainTries++;
                        continue;
                    }

                    // --- Packet Processing ---

                    std::string_view optionsView = packetView.substr(13, 2);
                    std::string_view checksumView = packetView.substr(15, 4);
                    std::string_view indexView = packetView.substr(19, 2);
                    std::string_view dataView = packetView.substr(21, expectedDataSize);

                    uint16_t options = static_cast<uint8_t>(optionsView[0]) +
                                       (static_cast<uint8_t>(optionsView[1]) << 8);

                    if (options & 0x1)
                    { // End of communication flag
                        fileStream.close();
                        uploadPath.remove();
                        SerialManager::sharedInstance->singleCommandLog(
                            NON_SHELL_MODE_NO_ERROR_CODE,
                            command.command_id
                        );
                        return;
                    }

                    uint32_t receivedChecksum =
                        static_cast<uint32_t>(static_cast<uint8_t>(checksumView[0])) |
                        (static_cast<uint32_t>(static_cast<uint8_t>(checksumView[1])) << 8) |
                        (static_cast<uint32_t>(static_cast<uint8_t>(checksumView[2])) << 16) |
                        (static_cast<uint32_t>(static_cast<uint8_t>(checksumView[3])) << 24);

                    uint16_t packetIndex = static_cast<uint8_t>(indexView[0]) +
                                           (static_cast<uint8_t>(indexView[1]) << 8);

                    uint32_t calculatedChecksum = 0;
                    for (unsigned char c : dataView)
                        calculatedChecksum = (calculatedChecksum + c) % 4294967295;

                    if (receivedChecksum != calculatedChecksum)
                    {
                        SerialManager::sharedInstance->singleCommandLog(
                            NON_SHELL_CHUNK_NEED_TRANSFER_AGAIN + std::to_string(packetIndex),
                            command.command_id
                        );
                        askAgainTries++;
                    }
                    else
                    {
                        fileStream.write(dataView.data(), dataView.size());
                        receivedSize += dataView.size();
                        SerialManager::sharedInstance->singleCommandLog(
                            NON_SHELL_MODE_NO_ERROR_CODE + std::to_string(packetIndex),
                            command.command_id
                        );
                        askAgainTries = 0;
                    }

                    tempBuffer.erase(0, packetView.size());
                }
            }

            fileStream.close();

            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(
                    "File " + uploadPath.str() + " with size " + std::to_string(fileSize) +
                    " uploaded successfully."
                );
            else
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
        }
        catch (const std::exception& e)
        {
            if (this->shellMode)
                SerialManager::sharedInstance->commandLog(
                    "Error uploading file: " + std::string(e.what())
                );
            else
                SerialManager::sharedInstance->commandLog(
                    NON_SHELL_MODE_ERROR_CODE + std::string(e.what())
                );

            uploadPath.remove();
            return;
        }
    }
} // namespace serialcom
