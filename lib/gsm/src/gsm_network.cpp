#include "gsm.hpp"
#include <URLResponse.hpp>
#include <threads.hpp>

#include <regex>
#include <iostream>

#define gsm Serial2

namespace GSM
{
    std::shared_ptr<network::URLSessionDataTask> currentRequest = nullptr;
    std::vector<std::shared_ptr<network::URLSessionDataTask>> hTTPRequests;

    void sendRequest(std::shared_ptr<network::URLSessionDataTask> request)
    {
        hTTPRequests.push_back(request);
        request->state = network::URLSessionDataTask::State::Waiting;

        request->updateTimeout();
    }

    size_t readResponseDataChunk(char* buffer) // read a single chunk of 1024 bytes
    {
        if (currentRequest == nullptr && currentRequest->response != std::nullopt)
            return 0;

        if(currentRequest->state != network::URLSessionDataTask::State::ResponseReceived)
            return 0;

        network::URLResponse requestResponse = currentRequest->response.value();

        #ifdef ESP_PLATFORM
        
        currentRequest->updateTimeout();

        GSM::coresync.lock();
        uint64_t timer = millis();  // pour le timeout
        uint64_t timeout_block = 3000;    // timeout de 3 secondes

        uint64_t nextBlockSize = std::min(uint64_t(1024), requestResponse.responseBodySize - requestResponse.readPosition);

        //std::cout << "Reading chunk: " << nextBlockSize << std::endl;

        gsm.println("AT+HTTPREAD=0,1024\r");

        //std::cout << "Waiting for the data" << std::endl;

        while (timer + timeout_block > millis() && (!gsm.available() || gsm.read() != 'K'));    // wait for the garbage data to be ignored
        while (timer + timeout_block > millis() && (!gsm.available() || gsm.read() != ':'));
        while (timer + timeout_block > millis() && (!gsm.available() || gsm.read() != '\n'));

        while (gsm.available() < nextBlockSize && timer + timeout_block > millis());

        if(gsm.available() < nextBlockSize)
        {
            std::cout << "Timeout" << std::endl;
            nextBlockSize = 0;
            closeRequest();
        }
        else
        {
            gsm.readBytes(buffer, nextBlockSize);
        }

        GSM::data += gsm.readString().c_str();

        //std::cout << "Chunk readed" << std::endl;
        
        requestResponse.readPosition += nextBlockSize;

        if (requestResponse.readPosition == requestResponse.responseBodySize)
        {
            closeRequest();
        }

        currentRequest->response = std::make_optional(requestResponse);

        GSM::coresync.unlock();
        return nextBlockSize;

        #endif

        return 0;
    }

    void closeRequest()
    {        
        if (currentRequest != nullptr)
        {
            if (currentRequest->state != network::URLSessionDataTask::State::Finished)
            {
                GSM::send("AT+HTTPTERM", "AT+HTTPTERM", 1000);
                currentRequest->state = network::URLSessionDataTask::State::Finished;
            }
            hTTPRequests.erase(std::remove(hTTPRequests.begin(), hTTPRequests.end(), currentRequest), hTTPRequests.end());
            currentRequest = nullptr;
        }
    }

    void requestsLoopCycle()
    {
        if(GSM::currentRequest == nullptr)
        {
            for (uint8_t i = 0; i < requests.size(); i++)
            {
                if(hTTPRequests[i]->state == network::URLSessionDataTask::State::Waiting)
                {
                    currentRequest = hTTPRequests[i];
                    break;
                }
            }
        }

        if(currentRequest != nullptr)
        {
            switch (currentRequest->state)
            {
                case network::URLSessionDataTask::State::Waiting:
                    if(currentRequest->isOverTimeout() || GSM::send("AT+HTTPINIT", "AT+HTTPINIT", 500).find("OK") == std::string::npos)   // setup http
                    {
                        killRequest();
                        break;
                    }
                    else
                    {
                        if(currentRequest->request.method == network::URLRequest::HTTPMethod::GET)
                        {
                            GSM::send("AT+HTTPPARA=\"URL\",\"" + currentRequest->request.url.absoluteString + "\"\r", "AT+HTTPPARA", 500);
                            if(GSM::send("AT+HTTPACTION=0", "AT+HTTPACTION", 5000).find("OK") == std::string::npos)
                            {
                                killRequest();
                                break;
                            }
                        } 
                        else if(currentRequest->request.method == network::URLRequest::HTTPMethod::POST)
                        {
                            // Set URL for POST request
                            GSM::send("AT+HTTPPARA=\"URL\",\"" + currentRequest->request.url.absoluteString + "\"\r", "AT+HTTPPARA", 500);
                            
                            // Set content type (assuming JSON, adjust if needed)
                            GSM::send("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r", "AT+HTTPPARA", 500);
                            
                            // Prepare to send data
                            int dataLength = currentRequest->request.httpBody.length();
                            GSM::send("AT+HTTPDATA=" + std::to_string(dataLength) + ",10000", "DOWNLOAD", 1000);
                            
                            // Send the actual POST data
                            GSM::send(currentRequest->request.httpBody, "OK", 500 + dataLength * 8 * BAUDRATE);  // wait for the full data transfer
                            
                            // Perform POST action
                            if(GSM::send("AT+HTTPACTION=1", "AT+HTTPACTION", 5000).find("OK") == std::string::npos)
                            {
                                killRequest();
                                break;
                            }
                        }

                        currentRequest->state = network::URLSessionDataTask::State::Running;

                        currentRequest->updateTimeout();
                    }
                    
                    break;
                case network::URLSessionDataTask::State::Running:
                    // let the key be received
                    if(currentRequest->isOverTimeout())
                    {
                        killRequest();
                    }
                    break;
                case network::URLSessionDataTask::State::ResponseReceived:
                    // let the app read the data
                    if(currentRequest->isOverTimeout())
                    {
                        closeRequest();
                        break;
                    }
                    break;
                case network::URLSessionDataTask::State::Cancelled:
                case network::URLSessionDataTask::State::Finished:
                    currentRequest = nullptr;
                    break;
            }
        }
    }

    void handleIncomingResponse()
    {
        std::regex pattern(".*\\+HTTPACTION: (\\d+),(\\d+),(\\d+)\\r\\n.*");
        std::smatch match;

        if(std::regex_search(GSM::data, match, pattern))
        {
            int action = std::stoi(match[1].str());
            int status = std::stoi(match[2].str());
            int size = std::stoi(match[3].str());

            std::cout << "action: " << action << " status: " << status << " size: " << size << std::endl;

            if(currentRequest != nullptr)
            {
                currentRequest->updateTimeout();
                currentRequest->handleResponse(status, size);
            }
        }
    }

    void killRequest(uint16_t code)
    {
        if(currentRequest != nullptr)
        {
            closeRequest();
            currentRequest->handleResponse(code, 0);
            currentRequest = nullptr;
        }
    }
}