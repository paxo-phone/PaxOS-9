#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include <string>
#include <shared_mutex>
#include <memory>

namespace network
{
    void init ();

    class NetworkManager 
    {
        public:
            NetworkManager();
            
            static std::shared_ptr<NetworkManager> sharedInstance;

            const void connectToWiFi(const std::string& ssid);

            const void connectToWiFi(const std::string& ssid, const std::string& passwd);

            const void disconnectWiFi();

            const void turnOFFWiFi();

            const void turnONWiFi();

            const std::string currentWiFiSSID();

            const bool isConnected();
        
        private:
    };
}

#endif // NETWORKMANAGER_HPP