#include "SerialManager.hpp"

#include "CommandsManager.hpp"

#include <array>
#include <cstddef>
#include <delay.hpp>
#include <iostream>
#include <streambuf>
#ifdef ESP_PLATFORM
#include <Arduino.h>
#endif

namespace serialcom
{
    const std::shared_ptr<SerialManager> SerialManager::sharedInstance =
        std::make_shared<SerialManager>();

    SerialManager::SerialManager()
    {
#ifdef ESP_PLATFORM
        Serial.begin(115200);
#endif

        std::ios_base::sync_with_stdio(false);

        this->coutBuffer.installOnStream(&std::cout);
        this->cerrBuffer.installOnStream(&std::cerr);
    }

    SerialManager::~SerialManager()
    {
#ifdef ESP_PLATFORM
        Serial.end();
#endif

        std::ios_base::sync_with_stdio(true);
    }

    void SerialManager::changeConsoleLockTo(bool newState)
    {
        this->consoleLocked = newState;
        this->coutBuffer.canFlushOnOverflow = !newState;
        this->cerrBuffer.canFlushOnOverflow = !newState;
    }

    void SerialManager::startCommandLog()
    {
        this->commandLogBuffer.fill(0);
        this->commandLogBufferIndex = 0;
        this->commandLogBufferHash = 0;
    }

    void SerialManager::commandLog(
        const std::string& message
    ) // should only be called in the serialLoop thread
    {
        // put in the commandLogBuffer

        uint64_t pseudoHash = this->commandLogBufferHash;

        for (char c : message)
            if (this->commandLogBufferIndex < MAX_OUTPUT_SIZE)
            {
                this->commandLogBuffer[this->commandLogBufferIndex] = c;
                this->commandLogBufferIndex++;
                pseudoHash = (pseudoHash + c) % 4294967295;
            } // TODO: handle overflows

        this->commandLogBufferHash = pseudoHash;
    }

    void SerialManager::singleCommandLog(
        const std::string& message, const char command_id[COMMAND_ID_SIZE]
    )
    {
        bool consoleState = this->getConsoleLockState();
        this->changeConsoleLockTo(true);
        SerialManager::sharedInstance->startCommandLog();
        SerialManager::sharedInstance->commandLog(message);
        SerialManager::sharedInstance->finishCommandLog(
            CommandsManager::defaultInstance->shellMode,
            command_id
        );
        this->changeConsoleLockTo(consoleState);
    }

    void SerialManager::finishCommandLog(bool shellMode, const char command_id[COMMAND_ID_SIZE])
    {
        if (shellMode)
        {
            this->coutBuffer.directLog(
                std::string(this->commandLogBuffer.data(), this->commandLogBufferIndex),
                true
            );
        }
        else
        {
            constexpr char startBytes[3] =
                {static_cast<char>(0xff), static_cast<char>(0xfe), static_cast<char>(0xfd)};
            std::string startBytesString(reinterpret_cast<const char*>(startBytes), 3);

            this->coutBuffer.directLog(startBytesString, false);

            std::string command_id_string(
                reinterpret_cast<const char*>(command_id),
                COMMAND_ID_SIZE
            );

            this->coutBuffer.directLog(command_id_string, false);

            std::string bufferIndex(
                reinterpret_cast<const char*>(&this->commandLogBufferIndex),
                sizeof(this->commandLogBufferIndex)
            );

            this->coutBuffer.directLog(bufferIndex, false);

            uint16_t options = 0b0000000000000000;
            std::string optionsString(reinterpret_cast<const char*>(&options), sizeof(options));

            this->coutBuffer.directLog(optionsString, false);

            uint32_t hash = this->commandLogBufferHash;

            std::string bufferHash(reinterpret_cast<const char*>(&hash), sizeof(hash));

            this->coutBuffer.directLog(bufferHash, false);

            this->coutBuffer.directLog(
                std::string(this->commandLogBuffer.data(), this->commandLogBufferIndex),
                false
            );
        }

        this->startCommandLog();
    }

    void SerialManager::forceFlushBuffers()
    {
        if (!this->consoleLocked)
        {
            this->coutBuffer.flushBuffer();
            this->cerrBuffer.flushBuffer();
        }
    }

    std::streambuf* SerialManager::changeDefaultCoutBuffer(std::streambuf* buffer)
    {
        return this->coutBuffer.changeDefaultBuffer(buffer);
    }

    std::streambuf* SerialManager::changeDefaultCerrBuffer(std::streambuf* buffer)
    {
        return this->cerrBuffer.changeDefaultBuffer(buffer);
    }

#ifdef ESP_PLATFORM
    void SerialManager::serialLoop(void*)
    {
#else
    void SerialManager::serialLoop()
    {
#endif
        SerialManager* serialManager = SerialManager::sharedInstance.get();
        std::cout << "Serial loop started" << std::endl;
        while (true)
        {
            serialManager->getInputCommand();
            if (serialManager->newData)
            {
                if (CommandsManager::defaultInstance->shellMode)
                    std::cout << std::endl;
                Command newCommand = Command(serialManager->current_input);
                serialManager->startCommandLog();
                CommandsManager::defaultInstance->processCommand(newCommand);
                serialManager->finishCommandLog(
                    CommandsManager::defaultInstance->shellMode,
                    newCommand.command_id
                );
                serialManager->newData = false;
            }

            if (!serialManager->consoleLocked)
            {
                serialManager->coutBuffer.flushBuffer();
                serialManager->cerrBuffer.flushBuffer();
            }

            PaxOS_Delay(10);
        }
    }

    void SerialManager::getInputCommand()
    {
        static size_t ndx = 0;
        char endMarker = '\n';
        char rc;

        while (this->isNewInputAvailable() && newData == false)
        {
#ifdef ESP_PLATFORM
            rc = Serial.read();
#else
            rc = std::cin.get();
#endif

            if (rc != endMarker)
            {
                current_input[ndx] = rc;
                ndx++;
                if (ndx >= INPUT_MAX_SIZE)
                    ndx = INPUT_MAX_SIZE - 1;
            }
            else
            {
                current_input[ndx] = '\0'; // terminate the string
                ndx = 0;
                newData = true;
            }
        }
    }

    bool SerialManager::isNewInputAvailable() const
    {
#ifdef ESP_PLATFORM
        return Serial.available() > 0;
#else
        return std::cin.peek() != EOF;
#endif
    }
} // namespace serialcom
