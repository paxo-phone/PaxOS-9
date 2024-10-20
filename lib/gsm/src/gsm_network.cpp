#include "gsm.hpp"
#include <threads.hpp>

#include <regex>
#include <iostream>

#define gsm Serial2

namespace GSM
{
    std::vector<HttpRequest*> HttpRequest::requests;
    HttpRequest* HttpRequest::currentRequest = nullptr;

    HttpRequest::HttpRequest(HttpHeader header) : header(header)
    {
    }

    HttpRequest::~HttpRequest()
    {
        close();
    }

    void HttpRequest::send(std::function <void (uint8_t, uint64_t)> callback)
    {
        if(callback == nullptr)
        {
            state = RequestState::ENDED;
        }

        requests.push_back(this);

        state = RequestState::WAITING;
        timeout = millis() + 10000;
        this->callback = callback;
    }

    size_t HttpRequest::readChunk(char* buffer) // read a single chunk of 1024 bytes
    {
        if(state != RequestState::RECEIVED)
            return 0;

        #ifdef ESP_PLATFORM
        
        timeout = millis() + 10000;

        GSM::coresync.lock();
        uint64_t timer = millis();  // pour le timeout
        uint64_t timeout_block = 3000;    // timeout de 1 secondes

        uint64_t nextBlockSize = std::min(uint64_t(1024), dataSize - readed);

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
            close();
        }
        else
        {
            gsm.readBytes(buffer, nextBlockSize);
        }

        GSM::data += gsm.readString().c_str();

        //std::cout << "Chunk readed" << std::endl;
        
        readed += nextBlockSize;

        if (readed == dataSize)
        {
            close();
        }

        GSM::coresync.unlock();
        return nextBlockSize;

        #endif

        return 0;
    }

    void HttpRequest::close()
    {
        if(state == RequestState::ENDED)
            return;
        
        GSM::send("AT+HTTPTERM", "AT+HTTPTERM", 1000);

        state = RequestState::ENDED;
        requests.erase(std::remove(requests.begin(), requests.end(), this), requests.end());
    }

    void HttpRequest::fastKill(uint8_t code)
    {
        close();
        callback(code, 0);
    }

    void HttpRequest::manage()
    {
        if(currentRequest == nullptr)
        {
            for (uint8_t i = 0; i < requests.size(); i++)
            {
                if(requests[i]->state == HttpRequest::RequestState::WAITING)
                {
                    currentRequest = requests[i];
                    break;
                }
            }
        }

        if(currentRequest != nullptr)
        {
            switch (currentRequest->state)
            {
                case HttpRequest::RequestState::WAITING:
                    if(GSM::send("AT+HTTPINIT", "AT+HTTPINIT", 500).find("OK") == std::string::npos)   // setup http
                    {
                        currentRequest->fastKill();
                        break;
                    }
                    else
                    {
                        if(currentRequest->header.httpMethod == HttpHeader::Method::GET)
                        {
                            GSM::send("AT+HTTPPARA=\"URL\",\"" + currentRequest->header.url + "\"\r", "AT+HTTPPARA", 500);
                            if(GSM::send("AT+HTTPACTION=0", "AT+HTTPACTION", 5000).find("OK") == std::string::npos)
                            {
                                currentRequest->fastKill();
                                break;
                            }
                        } 
                        else if(currentRequest->header.httpMethod == HttpHeader::Method::POST)
                        {
                            // Set URL for POST request
                            GSM::send("AT+HTTPPARA=\"URL\",\"" + currentRequest->header.url + "\"\r", "AT+HTTPPARA", 500);
                            
                            // Set content type (assuming JSON, adjust if needed)
                            GSM::send("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r", "AT+HTTPPARA", 500);
                            
                            // Prepare to send data
                            int dataLength = currentRequest->header.body.length();
                            GSM::send("AT+HTTPDATA=" + std::to_string(dataLength) + ",10000", "DOWNLOAD", 1000);
                            
                            // Send the actual POST data
                            GSM::send(currentRequest->header.body, "OK", 500 + dataLength * 8 * BAUDRATE);  // wait for the full data transfer
                            
                            // Perform POST action
                            if(GSM::send("AT+HTTPACTION=1", "AT+HTTPACTION", 5000).find("OK") == std::string::npos)
                            {
                                currentRequest->fastKill();
                                break;
                            }
                        }

                        currentRequest->state = HttpRequest::RequestState::SENT;
                    }
                    
                    break;
                case HttpRequest::RequestState::SENT:
                    // let the key be received
                    if(millis() > currentRequest->timeout)
                    {
                        currentRequest->fastKill(504);
                    }
                    break;
                case HttpRequest::RequestState::RECEIVED:
                    // let the app read the data
                    if(millis() > currentRequest->timeout)
                    {
                        currentRequest->close();
                        break;
                    }
                    break;
                case HttpRequest::RequestState::END:
                    currentRequest = nullptr;
                    break;
            }
        }
    }

    void HttpRequest::received()
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
                currentRequest->state = RequestState::RECEIVED;
                currentRequest->dataSize = size;
                currentRequest->timeout = millis() + 10000;
                currentRequest->callback(status, size);
            }
        }
    }
}