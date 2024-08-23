#include "../config.h"
#include "../CommandsManager.hpp"
#include "SerialManager.hpp"
#include <filestream.hpp>
#include <path.hpp>
#include <json.hpp>
#include <base64.hpp>
#include <string>
using json = nlohmann::json;

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
                SerialManager::sharedInstance->commandLog("Files and directories in " + firstArgument + ":");
                for (const auto& file : files) {
                    SerialManager::sharedInstance->commandLog(file);
                }
            } else {
                SerialManager::sharedInstance->commandLog("Directory " + firstArgument + " does not exist.");
            }
        } else {
            if (lsPath.exists()) {
                json output = json::array();
                output.push_back(files);
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
                SerialManager::sharedInstance->commandLog("File " + firstArgument + " already exists.");
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        } else {
            if(touchPath.newfile())
            {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                } else {
                    SerialManager::sharedInstance->commandLog("File " + firstArgument + " created.");
                }
            } else {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog("Error creating file " + firstArgument);
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
                SerialManager::sharedInstance->commandLog("Directory " + firstArgument + " already exists.");
            } else {
                SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
            }
        } else {
            if (mkdirPath.newdir())
            {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                } else {
                    SerialManager::sharedInstance->commandLog("Directory " + firstArgument + " created.");
                }
            } else {
                if (this->shellMode)
                {
                    SerialManager::sharedInstance->commandLog("Error creating directory " + firstArgument);
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
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                } else {
                    SerialManager::sharedInstance->commandLog("File or directory " + firstArgument + " removed.");
                }
            } else {
                if (this->shellMode) {
                    SerialManager::sharedInstance->commandLog("Error removing file or directory " + firstArgument);
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                }
            }
        } else {
            if (this->shellMode) {
                SerialManager::sharedInstance->commandLog("Error: " + firstArgument + " does not exist.");
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
                    SerialManager::sharedInstance->commandLog("Error: " + secondArgument + " already exist.");
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                }
            } else {
                if (cpDestinationPath.copyTo(cpDestinationPath))
                {
                    if (this->shellMode)
                    {
                        SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                    } else {
                        SerialManager::sharedInstance->commandLog("File or directory " + firstArgument + " copied to " + secondArgument);
                    }
                } else {
                    if (this->shellMode)
                    {
                        SerialManager::sharedInstance->commandLog("Error copying file or directory " + firstArgument + " to " + secondArgument);
                    } else {
                        SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                    }
                }
            }
        } else {
            if (this->shellMode) {
                SerialManager::sharedInstance->commandLog("Error: " + firstArgument + " does not exist.");
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
                    SerialManager::sharedInstance->commandLog("Error: " + secondArgument + " already exist.");
                } else {
                    SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                }
            } else {
                if (mvSourcePath.rename(mvDestinationPath))
                {
                    if (this->shellMode)
                    {
                        SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_NO_ERROR_CODE);
                    } else {
                        SerialManager::sharedInstance->commandLog("File or directory " + firstArgument + " moved to " + secondArgument);
                    }
                } else {
                    if (this->shellMode)
                    {
                        SerialManager::sharedInstance->commandLog("Error moving file or directory " + firstArgument + " to " + secondArgument);
                    } else {
                        SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
                    }
                }
            }
        } else {
            if (this->shellMode) {
                SerialManager::sharedInstance->commandLog("Error: " + firstArgument + " does not exist.");
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
            SerialManager::sharedInstance->commandLog("Error: command not implemented.");
        } else {
            SerialManager::sharedInstance->commandLog(NON_SHELL_MODE_ERROR_CODE);
        }
    }

    // arg1 = file to download
    // output = file content in chunks of 2048 bytes (or less for the last chunk), each one encoded in base64
    void CommandsManager::processDownloadCommand(const Command& command) const
    {
        storage::Path downloadPath = storage::Path(command.arguments[0]);

        std::cout << downloadPath.str() << std::endl << std::endl;
        storage::FileStream fileStream(downloadPath.str(), storage::READ);

        if (!fileStream.isopen())
        {
            SerialManager::sharedInstance->commandLog("Error: file couldn't be opened.");
            return;
        }
        #define BOOL_STR(b) (b?"true":"false")

        std::cout << "File size " << std::to_string(fileStream.size()) << " : " << std::to_string(fileStream.size() + 1) << " : " << BOOL_STR(fileStream.size() == 0) << std::endl; 

        if (fileStream.size() == 0)
        {
            SerialManager::sharedInstance->commandLog("Nothing in the file");
            return;
        }

        SerialManager::sharedInstance->commandLog(std::to_string(fileStream.size()));

        char nextChar = fileStream.readchar();
        while (nextChar != std::char_traits<char>::eof() && fileStream.sizeFromCurrentPosition() > 0)
        {
            SerialManager::sharedInstance->commandLog("chunk");
            SerialManager::sharedInstance->commandLog(std::to_string((uint8_t)nextChar));
            SerialManager::sharedInstance->commandLog("size till end");
            SerialManager::sharedInstance->commandLog(std::to_string(fileStream.sizeFromCurrentPosition()));
            std::string chunk = nextChar + fileStream.read(2047);
            std::string encodedChunk = base64::to_base64(chunk);
            SerialManager::sharedInstance->commandLog(encodedChunk);
            
            nextChar = fileStream.readchar();
        }

        SerialManager::sharedInstance->commandLog("finished");

    
        fileStream.close();
    }

    // arg1 = fileName to upload
    // arg2 = size of the file in bytes
    // input = file content in chunks of 2048 bytes (or less for the last chunk), each one encoded in base64
    void CommandsManager::processUploadCommand(const Command& command) const
    {
        storage::Path uploadPath(command.arguments[0]);
        SerialManager::sharedInstance->commandLog("Uploading file to " + uploadPath.str() + " with size " + command.arguments[1] + " bytes");
        size_t fileSize = std::stoi(command.arguments[1]);

        uploadPath.newfile();

        storage::FileStream fileStream(uploadPath.str(), storage::WRITE);

        for (size_t i = 0; i < fileSize; i += 2048)
        {
            std::string encodedChunk;
            std::cin >> encodedChunk;
            std::string chunk = base64::from_base64(encodedChunk).c_str();
            fileStream.write(chunk);
        }

        fileStream.close();
    }
} // namespace serialcom
