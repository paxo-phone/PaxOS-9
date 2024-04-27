#ifndef URLSESSION_HPP
#define URLSESSION_HPP

#include <shared_mutex>
#include <vector>
#include "URLSessionDataTask.hpp"

namespace network {
    class URLSession {
        public:
            static const std::shared_ptr<URLSession> defaultInstance;

            URLSession();

            URLSessionDataTask* dataTaskWithURL(const URL url, CompletionHandler completionHandler);
            URLSessionDataTask* dataTaskWithRequest(const URLRequest, CompletionHandler completionHandler);
    };
}
#endif // URLSESSION_HPP