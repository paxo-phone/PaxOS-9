#ifndef LUA_APP_
#define LUA_APP_

#include "SOL2/sol.hpp"

#include <gui.hpp>
#include <filestream.hpp>
#include <path.hpp>
#include <hardware.hpp>

#include "lua_gui.hpp"
#include "lua_hardware.hpp"
#include "lua_events.hpp"
#include "lua_storage.hpp"
#include "lua_gsm.hpp"
#include "lua_json.hpp"


/*class LuaHttpClient
{
    public:
    LuaHttpClient(LuaFile* lua);
    ~LuaHttpClient();
    std::string get(std::string url);
    std::string post(std::string url);

    network::HttpClient httpClient;

    private:
    LuaFile* lua;
};

class LuaNetwork
{
    public:
    LuaNetwork(LuaFile* lua);
    std::shared_ptr<LuaHttpClient> createHttpClient();

    private:
    LuaFile* lua;
};*/

struct Permissions
{
    bool acces_gui = true;
    bool acces_files = true;
    bool acces_files_root = true;
    bool acces_hardware = true;
    bool acces_time = true;
    bool acces_web_paxo = true;
    bool acces_web = true;
    bool acces_gsm = true;
};

class LuaFile
{
    public:
    LuaFile(storage::Path filename, storage::Path manifest);
    ~LuaFile();

    void run(std::vector<std::string> arg = {});

    Permissions perms;
    storage::Path directory;
    storage::Path manifest;

    sol::state lua;

    private:
    storage::Path filename;
    gui::elements::Window* current_root;

    LuaHardware lua_hardware;
    LuaGui lua_gui;
    LuaStorage lua_storage;
    LuaTime lua_time;
    //LuaNetwork lua_network;
};

#endif