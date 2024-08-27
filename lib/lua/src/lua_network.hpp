#ifndef LUA_NETWORK
#define LUA_NETWORK

#include "../network/network.hpp"

class LuaNetwork {

    public:
//        void callURL(std::string url, std::function<void(std::string)> completionHandler);
        void callURL(std::string url, std::function <void(std::string)> completionHandler, std::function <void(double)> progressHandler = nullptr);

};


#endif