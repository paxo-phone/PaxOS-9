#ifndef URLSESSIONDATATASK_HPP
#define URLSESSIONDATATASK_HPP

#include "URLSessionTask.hpp"
#include "URLRequest.hpp"
#include "CompletionHandler.hpp"
#include "ProgressHandler.hpp"

#include <iostream>
#include <curl/curl.h>


namespace network {
    class URLSessionDataTask : public URLSessionTask {
        public:
            URLSessionDataTask(const URLRequest request, CompletionHandler completionHandler);

            void resume();

            void cancel();

            void pause();

            std::string responseData;

            CompletionHandler completionHandler;

            ProgressHandler downloadProgressHandler;

            ProgressHandler uploadProgressHandler;

            uint64_t countOfBytesUploaded = 0; // will increase in case of POST request

            uint64_t countOfBytesExpectedToUpload = 0; // will increase in case of POST request

            double uploadProgress = 0;
        private:
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