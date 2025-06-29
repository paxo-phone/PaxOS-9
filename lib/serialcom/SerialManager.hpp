#pragma once

#include "config.h"

#include <ArrayedStreamBuffer.hpp>
#include <Command.hpp>
#include <ostream>
#include <shared_mutex>
#include <string>

namespace serialcom
{
    class SerialManager
    {
      public:
        SerialManager();
        ~SerialManager();

        static const std::shared_ptr<SerialManager> sharedInstance;

        void changeConsoleLockTo(bool newState);

        bool getConsoleLockState() const
        {
            return this->consoleLocked;
        }

        void startCommandLog();

        void commandLog(const std::string& message); // log by a command result only

        void singleCommandLog(
            const std::string& message, const char command_id[COMMAND_ID_SIZE] = {0}
        ); // log by a command result only

        void finishCommandLog(bool shellMode, const char command_id[COMMAND_ID_SIZE] = {0});

        void forceFlushBuffers();

        // Returns the previous buffer
        std::streambuf* changeDefaultCoutBuffer(std::streambuf* buffer);

        // Returns the previous buffer
        std::streambuf* changeDefaultCerrBuffer(std::streambuf* buffer);

#ifdef ESP_PLATFORM
        static void serialLoop(void*);
#else
        static void serialLoop();
#endif

      private:
        ArrayedStreamBuffer<MAX_OUTPUT_SIZE> coutBuffer;
        ArrayedStreamBuffer<MAX_OUTPUT_SIZE> cerrBuffer;

        std::array<char, MAX_OUTPUT_SIZE> commandLogBuffer = {
            '\0'
        }; // value-initialize buffer commandLogBuffer;
        uint16_t commandLogBufferIndex = 0;
        uint64_t commandLogBufferHash = 0;

        bool consoleLocked = false;
        bool newData = false;
        bool isOutputingCommand = false;
        char current_input[INPUT_MAX_SIZE] = {'\0'}; // an array to store the received data
        void getInputCommand();
        bool isNewInputAvailable() const;
    };
} // namespace serialcom
