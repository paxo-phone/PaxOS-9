#include "URLSessionDataTask.hpp"

#include "NetworkManager.hpp"

#include <string>
#include <map>

namespace network {
    URLSessionDataTask::URLSessionDataTask(const URLRequest request, CompletionHandler completionHandler) : URLSessionTask(request), completionHandler(completionHandler)
    {
        #if !defined(ESP32)
        curlHandle = curl_easy_init();
        #endif
    };

    void URLSessionDataTask::resume() {
        if (this->state != State::Suspended)
            return;

        if (!this->request.url.isValid())
        {
            this->state = State::Cancelled;
            return;
        }

        if (!NetworkManager::sharedInstance->isConnected())
        {
            this->state = State::Cancelled;
            return;
        }

        this->state = State::Running;

        #ifdef ESP32
            this->httpClient.begin(this->request.url.absoluteString.c_str());
            this->httpClient.setTimeout(this->request.timeoutInterval);
            
            for (const std::pair<const std::string, std::string>& header : this->request.httpHeaderFields)
            {
                this->httpClient.addHeader(header.first.c_str(), header.second.c_str());
            }

            int httpCode;

            switch (this->request.method)
            {
                case URLRequest::HTTPMethod::GET:
                    httpCode = this->httpClient.GET();
                    break;
                case URLRequest::HTTPMethod::POST:
                    httpCode = this->httpClient.POST(this->request.httpBody.c_str());
                    break;
                default:
                    throw std::runtime_error("Unsupported HTTP method");
            }

            if (httpCode == HTTP_CODE_OK) 
            {
                std::string payload = this->httpClient.getString().c_str();
                this->responseData = payload;
            } 

            this->httpClient.end();
            this->state = State::Completed;

        #else
            if (this->curlHandle == nullptr)
            {
                this->state = State::Cancelled;
                return;
            }
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

            this->state = State::Completed;
        #endif

        this->completionHandler(this->responseData);
    }

    void URLSessionDataTask::cancel() {
        if (this->state == State::Running)
        {
            #ifdef ESP32
                this->httpClient.end();
            #else
                curl_easy_cleanup(this->curlHandle);
            #endif
        }
        this->state = State::Cancelled;
    }

    void URLSessionDataTask::pause() { // not working right now
        if (this->state == State::Running)
        {
            #ifdef ESP32
                this->httpClient.end(); // has to be replaced by something that pause the task and doesn't cancels it
                this->state = State::Cancelled;
            #else
                curl_easy_pause(this->curlHandle, CURLPAUSE_ALL);
                this->state = State::Paused;
            #endif
        }
    }

    #if !defined(ESP32)
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
    #endif
}