#ifndef URLSESSION_HPP
#define URLSESSION_HPP

#include <shared_mutex>
#include <vector>
#include <memory>
#include "URLSessionDataTask.hpp"

namespace network {
    class URLSession {
        public:
            static const std::shared_ptr<URLSession> defaultInstance;

            bool preferWifi = false;

            bool concurrentRequestsLimit = 1;

            std::vector<std::shared_ptr<URLSessionDataTask>> tasks;

            URLSession();

            std::shared_ptr<URLSessionDataTask> dataTaskWithURL(const URL url, std::function<void(std::shared_ptr<URLSessionDataTask> task)> callback);
            std::shared_ptr<URLSessionDataTask> dataTaskWithRequest(const URLRequest, std::function<void(std::shared_ptr<URLSessionDataTask> task)> callback);
    };
}
#endif // URLSESSION_HPP