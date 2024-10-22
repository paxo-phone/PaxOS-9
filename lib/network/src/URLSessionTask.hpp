#ifndef URLSESSIONTASK_HPP
#define URLSESSIONTASK_HPP

#include "URLRequest.hpp"
#include "URLResponse.hpp"
#include <optional>
#include <clock.hpp>

namespace network {
    class URLSessionTask {
        public:
            enum State {
                Running,
                Waiting,
                Cancelled,
                Paused,
                ResponseReceived,
                Failed,
                Finished
            };

            const URLRequest request;

            std::optional<URLResponse> response;

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