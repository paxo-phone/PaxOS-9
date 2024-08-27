#include "NetworkManager.hpp"

#ifdef ESP_PLATFORM
    #include <WiFi.h>
#endif
#include <iostream>
#include <curl/curl.h>

namespace network
{
    const std::shared_ptr<NetworkManager> NetworkManager::sharedInstance = std::make_shared<NetworkManager>();

    NetworkManager::NetworkManager()
    {
        #ifdef ESP_PLATFORM
            this->turnON();
        #endif
        curl_global_init(CURL_GLOBAL_ALL);
    }

    const void NetworkManager::connect(const std::string& ssid)
    {
        #ifdef ESP_PLATFORM
            WiFi.begin(ssid.c_str());
        #endif
    }

    const void NetworkManager::connect(const std::string& ssid, const std::string& password)
    {
        #ifdef ESP_PLATFORM
            WiFi.begin(ssid.c_str(), password.c_str());
        #endif
    }

    const void NetworkManager::disconnect(void)
    {
        #ifdef ESP_PLATFORM
            WiFi.disconnect();
        #endif
    }

    const void NetworkManager::turnOFF(void)
    {
        #ifdef ESP_PLATFORM
            WiFi.mode(WIFI_OFF);
        #endif
    }

    const void NetworkManager::turnON(void)
    {
        #ifdef ESP_PLATFORM
            WiFi.mode(WIFI_STA);
        #endif
    }

    const bool NetworkManager::isConnected()
    {
        #ifdef ESP_PLATFORM
            if (WiFi.status() != WL_CONNECTED)
                return false;
        #else
        
        int pingExitCode = system("ping -c1 -s1 8.8.8.8 > /dev/null 2>&1"); // ping Google's DNS server and return the exit code
        if (pingExitCode == 0)
        {
            return true;
        } else
        {
            return false;
        }
       #endif
    }
}