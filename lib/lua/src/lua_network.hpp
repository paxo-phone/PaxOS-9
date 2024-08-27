#ifndef LUA_NETWORK
#define LUA_NETWORK

#include "../network/network.hpp"

class LuaNetwork {

    public:
        void callURL(std::string url, std::function<void(std::string)> completionHandler);

};


#endif