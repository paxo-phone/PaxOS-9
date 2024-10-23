#include "URLSessionDataTask.hpp"

#include "NetworkManager.hpp"

#include <gsm.hpp>

#include <string>
#include <map>

namespace network {
    URLSessionDataTask::URLSessionDataTask(const URLRequest request, bool useWiFi, std::function<void(std::shared_ptr<URLSessionDataTask> task)> callback) : URLSessionTask(request, useWiFi), callback(callback)
    {
        #ifdef ESP_PLATFORM
            if (this->useWiFi)
            {
                curlHandle = curl_easy_init();
            }
        #else
            curlHandle = curl_easy_init();
        #endif

        if (this->state != State::Waiting)
            return;

        if (!this->request.url.isValid())
        {
            this->state = State::Cancelled;
            return;
        }

        if (!NetworkManager::sharedInstance->isConnected())
        {
            std::cout << "No internet connection" << std::endl;
            this->state = State::Cancelled;
            return;
        }

        #ifdef ESP_PLATFORM
            if (this->useWiFi)
            {
                if (this->curlHandle == nullptr)
                {
                    this->state = State::Cancelled;
                    return;
                }
                this->sendRequestWiFi();
            }
            else
            {
                this->sendRequestGSM();
            }
        #else
            this->sendRequestWiFi();
        #endif
    };

    void URLSessionDataTask::handleResponse(uint16_t statusCode, uint64_t responseBodySize)
    {
        this->response = URLResponse(statusCode, responseBodySize);

        if (statusCode == 200)
        {
            this->state = State::ResponseReceived;
        }
        else
        {
            this->state = State::Failed;
        }

        this->callback(std::make_shared<URLSessionDataTask>(*this));
    }

    void URLSessionDataTask::sendRequestGSM()
    {
        std::cout << "sendRequestGSM" << std::endl;
        GSM::sendRequest(std::make_shared<URLSessionDataTask>(*this));
    }

    void URLSessionDataTask::sendRequestWiFi()
    {
        std::cout << "sendRequestWIFI" << std::endl;

        this->state = State::Running;

        switch (this->request.method)
        {
            case URLRequest::HTTPMethod::GET:
                curl_easy_setopt(this->curlHandle, CURLOPT_HTTPGET, 1L);
                break;
            case URLRequest::HTTPMethod::POST:
                curl_easy_setopt(this->curlHandle, CURLOPT_POSTFIELDSIZE, this->request.httpBody.length());
                curl_easy_setopt(this->curlHandle, CURLOPT_POSTFIELDS, this->request.httpBody.c_str());
                break;
            default:
                throw std::runtime_error("Unsupported HTTP method");
        }
        curl_easy_setopt(this->curlHandle, CURLOPT_URL, this->request.url.absoluteString.c_str());
        curl_easy_setopt(this->curlHandle, CURLOPT_WRITEFUNCTION, this->WriteCallback);
        curl_easy_setopt(this->curlHandle, CURLOPT_WRITEDATA, &this->responseData);
        curl_easy_setopt(this->curlHandle, CURLOPT_TIMEOUT_MS, this->request.timeoutInterval);

        struct progress_s data = {this, 0}; /* pass struct to callback  */

        curl_easy_setopt(this->curlHandle, CURLOPT_XFERINFODATA, &data);
        curl_easy_setopt(this->curlHandle, CURLOPT_XFERINFOFUNCTION, this->progress_callback);

        curl_easy_setopt(this->curlHandle, CURLOPT_NOPROGRESS, 0L);

        // set http headers
        struct curl_slist* headers = nullptr;
        for (const std::pair<const std::string, std::string>& header : this->request.httpHeaderFields)
        {
            headers = curl_slist_append(headers, (header.first + ": " + header.second).c_str());
        }
        curl_easy_setopt(this->curlHandle, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(this->curlHandle);

        if (res != CURLE_OK) 
        {
            std::cerr << "cURL error: " << curl_easy_strerror(res) << std::endl;
        }

        uint64_t httpCode = 0;
        curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &httpCode);

        this->handleResponse(httpCode, this->responseData.length());
    }

    void URLSessionDataTask::resume() {
        if (this->state == State::Paused)
        {
            if (this->useWiFi)
            {
                curl_easy_pause(this->curlHandle, CURLPAUSE_CONT);
            }
            else
            {
                //GSM::resumeRequest(); TODO
            }
        }
    }

    void URLSessionDataTask::cancel() {
        if (this->state == State::Running)
        {
            curl_easy_cleanup(this->curlHandle);
        }
        this->state = State::Cancelled;
    }

    void URLSessionDataTask::pause() { // not working right now
        if (this->state == State::Running)
        {
            if (this->useWiFi)
            {
                curl_easy_pause(this->curlHandle, CURLPAUSE_ALL);
            }
            else
            {
                //GSM::pauseRequest(); TODO
            }
            this->state = State::Paused;
        }
    }

    size_t URLSessionDataTask::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output)
    {
        size_t total_size = size * nmemb;
        output->append(static_cast<char*>(contents), total_size);
        return total_size;
    } 

    size_t URLSessionDataTask::progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
    {
        struct progress_s *memory = static_cast<struct progress_s*>(clientp);

        memory->task->countOfBytesReceived = dlnow;
        memory->task->countOfBytesExpectedToReceive = dltotal;
        memory->task->countOfBytesUploaded = ulnow;
        memory->task->countOfBytesExpectedToUpload = ultotal;


        if (dltotal == 0)
        {
            memory->task->progress = 0;
        } else {
            memory->task->progress = double(dlnow) / double(dltotal);
        }

        if (memory->task->downloadProgressHandler)
        {
            memory->task->downloadProgressHandler(memory->task->progress);
        }


        if (ultotal == 0)
        {
            memory->task->uploadProgress = 0;
        } else {
            memory->task->uploadProgress = double(ulnow) / double(ultotal);
        }

        if (memory->task->uploadProgressHandler)
        {
            memory->task->uploadProgressHandler(memory->task->uploadProgress);
        }                
        return 0; /* all is good */
    }

    size_t URLSessionDataTask::readChunk(char* buffer)
    {
        if (this->state != State::ResponseReceived)
            return 0;

        #ifdef ESP_PLATFORM
            if (this->useWiFi)
            {
                if (this->responseData.empty())
                    return 0;
                return this->responseData.copy(buffer, this->response->responseBodySize); // TODO read only a chunk like GSM does
            }
            else
            {
                return GSM::readResponseDataChunk(buffer);
            }
        #else
            if (this->responseData.empty())
                return 0;
            return this->responseData.copy(buffer, this->response->responseBodySize);
        #endif
    }
}