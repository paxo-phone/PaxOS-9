#include "SerialManager.hpp"
#include "CommandsManager.hpp"
#ifdef ESP_PLATFORM
#include <Arduino.h>
#else
#include <iostream>
#endif

namespace serialcom {
    const std::shared_ptr<SerialManager> SerialManager::sharedInstance = std::make_shared<SerialManager>();

    SerialManager::SerialManager() {
        #ifdef ESP_PLATFORM
        Serial.begin(115200);
        #endif
    }

    SerialManager::~SerialManager() {
        #ifdef ESP_PLATFORM
        Serial.end();
        #endif
    }

    void SerialManager::log(std::string message) {
        // copy the string into the outputbuffer from the outputesize position and then increment the outputsize
        size_t messageSize = message.size();

        if (this->outputSize + messageSize + 1 > MAX_OUTPUT_SIZE) {
            if (messageSize + 1 > MAX_OUTPUT_SIZE) { // truncate message
                message = message.substr(messageSize - MAX_OUTPUT_SIZE - 1, MAX_OUTPUT_SIZE - 1);
                messageSize = MAX_OUTPUT_SIZE - 1;
            } else {
                this->outputSize = MAX_OUTPUT_SIZE - messageSize - 1; // truncate the output buffer
            }
        }

        for (size_t i = 0; i < messageSize; i++) {
            this->outputBuffer[outputSize - 1 + i] = message[i];
        }
        
        this->outputBuffer[outputSize + messageSize] = '\n';

        this->outputSize += messageSize + 1;
    }

    void SerialManager::flushOutputBuffer() {
        if (this->outputSize == 0) {
            return;
        }
        this->println(std::string(this->outputBuffer));
        this->outputSize = 0;
    }

    void SerialManager::commandLog(const std::string& message) const { // should only be called in the serialLoop thread
        this->println(message);
    }

    #ifdef ESP_PLATFORM
    void SerialManager::serialLoop(void *) {
    #else
    void SerialManager::serialLoop() {
    #endif
        SerialManager *serialManager = SerialManager::sharedInstance.get();
        serialManager->println("Serial loop started");
        serialManager->getInputCommand();
        if (serialManager->newData) {
            Command newCommand = Command(serialManager->current_input);
            CommandsManager::defaultInstance->processCommand(newCommand);
            serialManager->newData = false;
        }

        serialManager->flushOutputBuffer();
    }

    void SerialManager::getInputCommand() {
        static size_t ndx = 0;
        char endMarker = '\n';
        char rc;
        
        while (this->isNewInputAvailable() && newData == false) {
            #ifdef ESP_PLATFORM
            rc = Serial.read();
            #else
            rc = std::cin.get();
            #endif

            if (rc != endMarker) {
                current_input[ndx] = rc;
                ndx++;
                if (ndx >= INPUT_MAX_SIZE) {
                    ndx = INPUT_MAX_SIZE - 1;
                } 
            }
            else {
                current_input[ndx] = '\0'; // terminate the string
                ndx = 0;
                newData = true;
            }
        }
    }

    void SerialManager::println(const std::string& log) const {
        #ifdef ESP_PLATFORM
        Serial.println(log.c_str());
        #else
        std::cout << log << std::endl;
        #endif
    }

    bool SerialManager::isNewInputAvailable() const {
        #ifdef ESP_PLATFORM
        return Serial.available() > 0;
        #else
        return std::cin.peek() != EOF;
        #endif
    }
}