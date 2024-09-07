#pragma once

#include <string>
#include <ArrayedStreamBuffer.hpp>
#include <Command.hpp>
#include <shared_mutex>
#include <ostream>
#include "config.h"

namespace serialcom {
    class SerialManager
    {
    public:
        SerialManager();
        ~SerialManager();

        static const std::shared_ptr<SerialManager> sharedInstance;

        void commandLog(const std::string& message); // log by a command result only

        void forceFlushBuffers();

        // Returns the previous buffer
        std::streambuf* changeDefaultCoutBuffer(std::streambuf* buffer);

        // Returns the previous buffer
        std::streambuf* changeDefaultCerrBuffer(std::streambuf* buffer);

        #ifdef ESP_PLATFORM
        static void serialLoop(void *);
        #else
        static void serialLoop();
        #endif

        private:
        ArrayedStreamBuffer<MAX_OUTPUT_SIZE> coutBuffer;
        ArrayedStreamBuffer<MAX_OUTPUT_SIZE> cerrBuffer;

        bool newData = false;
        bool isOutputingCommand = false;
        char current_input[INPUT_MAX_SIZE] = {'\0'};   // an array to store the received data
        void getInputCommand();
        bool isNewInputAvailable() const;
    };
}