#include "SerialManager.hpp"
#include "CommandsManager.hpp"
#include <delay.hpp>
#include <iostream>
#ifdef ESP_PLATFORM
#include <Arduino.h>
#endif

namespace serialcom {
    const std::shared_ptr<SerialManager> SerialManager::sharedInstance = std::make_shared<SerialManager>();

    SerialManager::SerialManager() {
        #ifdef ESP_PLATFORM
        Serial.begin(115200);
        #endif

        this->buffer.installOnStream(&std::cout);
    }

    SerialManager::~SerialManager() {
        #ifdef ESP_PLATFORM
        Serial.end();
        #endif
    }
    
    void SerialManager::commandLog(const std::string& message) { // should only be called in the serialLoop thread
        this->buffer.directLog(message);
    }

    #ifdef ESP_PLATFORM
    void SerialManager::serialLoop(void *) {
    #else
    void SerialManager::serialLoop() {
    #endif
        SerialManager *serialManager = SerialManager::sharedInstance.get();
        std::cout << "Serial loop started" << std::endl;
        while (true) {
            serialManager->getInputCommand();
            if (serialManager->newData) {
                Command newCommand = Command(serialManager->current_input);
                CommandsManager::defaultInstance->processCommand(newCommand);
                serialManager->newData = false;
            }

            serialManager->buffer.flushBuffer();

            PaxOS_Delay(10);
        }
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

    bool SerialManager::isNewInputAvailable() const {
        #ifdef ESP_PLATFORM
        return Serial.available() > 0;
        #else
        return std::cin.peek() != EOF;
        #endif
    }
}