#include "NetworkManager.hpp"

#ifdef ESP_PLATFORM
    #include <WiFi.h>
#endif
#include <iostream>
#include <curl/curl.h>
#include <gsm.hpp>

#include <URLSession.hpp>

namespace network
{
    std::shared_ptr<NetworkManager> NetworkManager::sharedInstance/* = std::make_shared<NetworkManager>()*/;

    void init()
    {
        NetworkManager::sharedInstance = std::make_shared<NetworkManager>();
        URLSession::defaultInstance = std::make_shared<URLSession>();
    }

    NetworkManager::NetworkManager()
    {
        #ifdef ESP_PLATFORM
            this->turnOFFWiFi();
        #endif
        curl_global_init(CURL_GLOBAL_ALL);
    }

    const void NetworkManager::connectToWiFi(const std::string& ssid)
    {
        #ifdef ESP_PLATFORM
            WiFi.begin(ssid.c_str());
        #endif
    }

    const void NetworkManager::connectToWiFi(const std::string& ssid, const std::string& password)
    {
        #ifdef ESP_PLATFORM
            WiFi.begin(ssid.c_str(), password.c_str());
        #endif
    }

    const void NetworkManager::disconnectWiFi(void)
    {
        #ifdef ESP_PLATFORM
            WiFi.disconnect();
        #endif
    }

    const void NetworkManager::turnOFFWiFi(void)
    {
        #ifdef ESP_PLATFORM
            WiFi.mode(WIFI_OFF);
        #endif
    }

    const void NetworkManager::turnONWiFi(void)
    {
        #ifdef ESP_PLATFORM
            WiFi.mode(WIFI_STA);
        #endif
    }

    const std::string NetworkManager::currentWiFiSSID()
    {
        #ifdef ESP_PLATFORM
            return WiFi.SSID().c_str();
        #else
            return "";
        #endif
    }

    const bool NetworkManager::isConnected()
    {
        return this->isWiFiConnected() || this->isGSMConnected();
    }

    const bool NetworkManager::isWiFiConnected()
    {
        #ifdef ESP_PLATFORM
            return WiFi.status() == WL_CONNECTED;
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

    const bool NetworkManager::isGSMConnected()
    {
        #ifdef ESP_PLATFORM
            return GSM::getNetworkStatus() > 0;
        #else
            return false;
        #endif
    }
}