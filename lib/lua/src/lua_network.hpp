#ifndef LUA_NETWORK_HPP
#define LUA_NETWORK_HPP

#include "SOL2/sol.hpp"
#include "network.hpp"

#include <memory>

class LuaFile; // Forward declaration

class LuaHttpClient
{
  public:
    LuaHttpClient(LuaFile* lua);
    ~LuaHttpClient();

    void get(const std::string& url, sol::table callbacks);
    void post(const std::string& url, const std::string& body, sol::table options);

  private:
    LuaFile* m_lua;
};

class LuaNetwork
{
  public:
    LuaNetwork(LuaFile* lua);
    std::shared_ptr<LuaHttpClient> createHttpClient();

  private:
    LuaFile* m_lua;
};

#endif // LUA_NETWORK_HPP
