#ifndef URLSESSIONTASK_HPP
#define URLSESSIONTASK_HPP

#include "URLRequest.hpp"
#include "URLResponse.hpp"
#include <optional>
#include <clock.hpp>
#include <cstdint>

namespace network {
    class URLSessionTask {
        public:
            enum State {
                /*
                * The task is waiting to be launched
                */
                Waiting,

                /*
                * The task is sending/has been sent to the server
                */
                Running,

                /*
                * The task has been cancelled by the user or by the system because of an internal error (see return code)
                */
                Cancelled,

                /*
                * The task has been paused by the user, not effective when useWiFi is false
                */
                Paused,

                /*
                * The task has received a response from the server, you can read the response in the `response` property
                */
                ResponseReceived,

                /*
                * The task has finished, no data can't be read anymore (except when useWiFi is true)
                */
                Finished
            };

            const URLRequest request;

            /*
            * The response of the request

            @note Will be empty until a response from the server is received or an internal error (timout, invalid url, etc...) happens.
            */
            std::optional<URLResponse> response;

            /*
            * Boolean indicating whether the task is using WiFi or GSM
            */
            const bool useWiFi;

            virtual void resume() = 0;

            virtual void cancel() = 0;

            virtual void pause() = 0;

            State state = State::Waiting;

            double progress = 0.0;

            uint64_t countOfBytesReceived = 0;
            uint64_t countOfBytesExpectedToReceive = 0; // total bytes expected to be received

            void updateTimeout() {
                this->absoluteTimeout = millis() + this->request.timeoutInterval;
            }

            bool isOverTimeout() {
                return millis() > this->absoluteTimeout && this->absoluteTimeout != -1;
            }
        protected:
            URLSessionTask(const URLRequest request, bool useWiFi) : request(request), useWiFi(useWiFi) {};

            virtual ~URLSessionTask() { } 

            uint64_t absoluteTimeout = -1;
    };
}

#endif // URLSESSIONTASK_HPP