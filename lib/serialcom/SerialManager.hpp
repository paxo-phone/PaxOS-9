#pragma once

#include <string>
#include <Command.hpp>
#include <shared_mutex>
#include "config.h"

namespace serialcom {
    class SerialManager
    {
    public:
        SerialManager();
        ~SerialManager();

        static const std::shared_ptr<SerialManager> sharedInstance;

        void log(std::string message);

        void commandLog(const std::string& message) const;

        #ifdef ESP_PLATFORM
        static void serialLoop(void *);
        #else
        static void serialLoop();
        #endif

        private:

        char outputBuffer[MAX_OUTPUT_SIZE] = {'\0'};
        uint32_t outputSize = 0;

        bool newData = false;
        bool isOutputingCommand = false;
        char current_input[INPUT_MAX_SIZE];   // an array to store the received data
        void flushOutputBuffer();
        void getInputCommand();
        void println(const std::string& log) const;
        bool isNewInputAvailable() const;
    };
}