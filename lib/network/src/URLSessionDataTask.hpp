#ifndef URLSESSIONDATATASK_HPP
#define URLSESSIONDATATASK_HPP

#include "URLSessionTask.hpp"
#include "URLRequest.hpp"
#include "ProgressHandler.hpp"
#include <memory>

#include <cstdint>
#include <iostream>
#include <curl/curl.h>


namespace network {
    class URLSessionDataTask : public URLSessionTask {
        public:
            URLSessionDataTask(const URLRequest request, bool useWiFi, std::function<void(std::shared_ptr<URLSessionDataTask>)> callback);

            bool useWiFi;

            void resume();

            /*
            * Cancel the task

            */
            void cancel();

            /*
            * Pause the task

            * @note Has no effect when useWifi is false
            */
            void pause();

            /*
            * Handle the response of the request

            * @param statusCode The status code of the response
            * @param responseBodySize The size of the response body
            */
            void handleResponse(uint16_t statusCode, uint64_t responseBodySize);

            /*
            * Read a chunk of the response data

            @param buffer The buffer to write the data to
            @return The number of bytes read

            @note Will only work if the task is in the ResponseReceived state
            */
            size_t readChunk(char* buffer);

            /*
            * The response of the request

            @note Will be empty until a response from the server is received or an internal error (timout, invalid url, etc...) happens.
            */
            std::optional<URLResponse> response;


            /*
            * The progress of the download

            @note Will only work is useWiFi is true
            */
            ProgressHandler downloadProgressHandler;

            /*
            * The progress of the upload

            @note Will only work is useWiFi is true
            */
            ProgressHandler uploadProgressHandler;

            /*
            * The time when the task will timeout (in ms)
            */
            uint64_t absoluteTimeout = -1;

            uint64_t countOfBytesUploaded = 0; // will increase in case of POST request

            uint64_t countOfBytesExpectedToUpload = 0; // will increase in case of POST request

            double uploadProgress = 0;

            /*
            * The callback function to call when the task has received a response and you now can read the data or when an error occurs
            */
            std::function<void(std::shared_ptr<URLSessionDataTask> task)> callback;

        private:
            void sendRequestGSM();

            void sendRequestWiFi();

            CURL* curlHandle;

            std::string responseData;

            /*
            * Callback function for cURL to write the response data
            */
            static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

            struct progress_s {
                URLSessionDataTask* task;
                size_t size;
            };
            
            /*
            * Callback function for cURL to update the progress

            @note This function is called by cURL and represents the progress of the data transfer. When using GSM (i.e. useWiFi is false), 
            the progress is defined by the size of the response body and the readPosition of the response (so no need for a progress callback).
            */
            static size_t progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
    };
}

#endif // URLSESSIONDATATASK_HPP