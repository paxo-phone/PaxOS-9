#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include <string>
#include <shared_mutex>
#include <memory>

namespace network
{
    class NetworkManager 
    {
        public:
            NetworkManager();
            
            static const std::shared_ptr<NetworkManager> sharedInstance;

            const void connectToWiFi(const std::string& ssid);

            const void connectToWiFi(const std::string& ssid, const std::string& passwd);

            const void disconnectWiFi();

            const void turnOFFWiFi();

            const void turnONWiFi();

            const std::string currentWiFiSSID();

            const bool isConnected();
        
        private:
            static NetworkManager _sharedInstance;
    };
}

#endif // NETWORKMANAGER_HPP