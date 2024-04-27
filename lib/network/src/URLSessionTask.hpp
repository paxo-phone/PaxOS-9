#ifndef URLSESSIONTASK_HPP
#define URLSESSIONTASK_HPP

#include "URLRequest.hpp"

namespace network {
    class URLSessionTask {
        public:
            enum State {
                Running,
                Suspended,
                Cancelled,
                Paused,
                Completed
            };

            const URLRequest request;

            virtual void resume() = 0;

            virtual void cancel() = 0;

            virtual void pause() = 0;

            State state = State::Suspended;

            double progress = 0.0;

            uint64_t countOfBytesReceived = 0;
            uint64_t countOfBytesExpectedToReceive = 0; // total bytes expected to be received

        protected:
            URLSessionTask(const URLRequest request) : request(request) {};

            virtual ~URLSessionTask() { } 
    };
}

#endif // URLSESSIONTASK_HPP