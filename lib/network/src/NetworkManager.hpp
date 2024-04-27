#ifndef NETWORKMANAGER_HPP
#define NETWORKMANAGER_HPP

#include <string>
#include <shared_mutex>

namespace network
{
    class NetworkManager 
    {
        public:
            NetworkManager();
            
            static const std::shared_ptr<NetworkManager> sharedInstance;

            const void connect(const std::string& ssid);

            const void connect(const std::string& ssid, const std::string& passwd);

            const void disconnect();

            const void turnOFF();

            const void turnON();

            const bool isConnected();
    };
}

#endif // NETWORKMANAGER_HPP