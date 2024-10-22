#include "URLSession.hpp"
#include "NetworkManager.hpp"
#include "URLSessionDataTask.hpp"

namespace network {
    std::shared_ptr<URLSession> URLSession::defaultInstance/* = std::make_shared<URLSession>()*/;

    URLSession::URLSession() {
        NetworkManager::sharedInstance->turnONWiFi();
    }
    
    std::shared_ptr<URLSessionDataTask> URLSession::dataTaskWithURL(const URL url, std::function<void(std::shared_ptr<URLSessionDataTask> task)> callback)
    {
        URLRequest request(url);
        return this->dataTaskWithRequest(request, callback);
    }

    std::shared_ptr<URLSessionDataTask> URLSession::dataTaskWithRequest(const URLRequest request, std::function<void(std::shared_ptr<URLSessionDataTask> task)> callback)
    {
        std::shared_ptr<URLSessionDataTask> task = std::make_shared<URLSessionDataTask>(URLSessionDataTask(request, this->preferWifi, callback));
        this->tasks.push_back(task);
        return task;
    }
}