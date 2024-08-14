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

        #ifdef ESP_PLATFORM
        static void serialLoop(void *);
        #else
        static void serialLoop();
        #endif

        private:
        ArrayedStreamBuffer<MAX_OUTPUT_SIZE> buffer;

        bool newData = false;
        bool isOutputingCommand = false;
        char current_input[INPUT_MAX_SIZE];   // an array to store the received data
        void getInputCommand();
        bool isNewInputAvailable() const;
    };
}