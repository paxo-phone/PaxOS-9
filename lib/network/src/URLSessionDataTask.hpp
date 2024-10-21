#ifndef URLSESSIONDATATASK_HPP
#define URLSESSIONDATATASK_HPP

#include "URLSessionTask.hpp"
#include "URLRequest.hpp"
#include "ProgressHandler.hpp"
#include <memory>

#include <iostream>
#include <curl/curl.h>


namespace network {
    class URLSessionDataTask : public URLSessionTask {
        public:
            URLSessionDataTask(const URLRequest request, bool useWiFi, std::function<void(std::shared_ptr<URLSessionDataTask>)> callback);

            bool useWiFi;

            void resume();

            void cancel();

            void pause();

            void handleResponse(uint16_t statusCode, uint64_t responseBodySize);

            size_t readChunk(char* buffer);

            std::optional<URLResponse> response;

            std::string responseData;

            ProgressHandler downloadProgressHandler;

            ProgressHandler uploadProgressHandler;

            uint64_t absoluteTimeout = -1;

            uint64_t countOfBytesUploaded = 0; // will increase in case of POST request

            uint64_t countOfBytesExpectedToUpload = 0; // will increase in case of POST request

            double uploadProgress = 0;

            std::function<void(std::shared_ptr<URLSessionDataTask> task)> callback; // TODO: check if it doesn't cause a memory leak

        private:
            void sendRequestGSM();

            void sendRequestWiFi();

            CURL* curlHandle;

            static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

            struct progress_s {
                URLSessionDataTask* task;
                size_t size;
            };
            
            static size_t progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
    };
}

#endif // URLSESSIONDATATASK_HPP