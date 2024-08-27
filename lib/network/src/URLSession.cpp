#include "URLSession.hpp"
#include "NetworkManager.hpp"
#include "URLSessionDataTask.hpp"
#include "CompletionHandler.hpp"

namespace network {
    const std::shared_ptr<URLSession> URLSession::defaultInstance = std::make_shared<URLSession>();

    URLSession::URLSession() {
        NetworkManager::sharedInstance->turnON();
    }
    
    URLSessionDataTask* URLSession::dataTaskWithURL(const URL url, CompletionHandler completionHandler)
    {
        URLRequest request(url);
        return this->dataTaskWithRequest(request, completionHandler);
    }

    URLSessionDataTask* URLSession::dataTaskWithRequest(const URLRequest request, CompletionHandler completionHandler)
    {
        return new URLSessionDataTask(request, completionHandler);
    }
}