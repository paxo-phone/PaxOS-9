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
#include "lua_network.hpp"


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

    void load();
    void runBackground(std::vector<std::string> arg = {});
    void run(std::vector<std::string> arg = {});
    void wakeup(std::vector<std::string> arg = {});
    void stop(std::vector<std::string> arg = {});

    void loop();

        sol::protected_function oncall; 
        sol::protected_function onlowbattery;
        sol::protected_function oncharging;
        sol::protected_function onmessage;
        sol::protected_function onmessageerror;
    
        void event_oncall() { if(oncall.valid()) oncall(); }
        void event_onlowbattery() { if(onlowbattery.valid()) onlowbattery(); }
        void event_oncharging() { if(oncharging.valid()) oncharging(); }
        void event_onmessage() {
            if(onmessage.valid()) {
                sol::protected_function_result result = onmessage();
                if (!result.valid()) {
                    sol::error err = result;
                    std::cout << "[LuaFile] onmessage event error: " << err.what() << std::endl;
                } else {
                    std::cout << "onmessage event activated" << std::endl;
                }
            }
        }

        void event_onmessageerror()
        {
            if(onmessage.valid()) {
                sol::protected_function_result result = onmessageerror();
                if (!result.valid()) {
                    sol::error err = result;
                    std::cout << "[LuaFile] onmessage event error: " << err.what() << std::endl;
                } else {
                    std::cout << "onmessage event activated" << std::endl;
                }
            }
        }

    Permissions perms;
    storage::Path directory;
    storage::Path manifest;
    sol::state lua;

    storage::Path filename;
    gui::elements::Window* current_root;

    LuaHardware lua_hardware;
    LuaGui lua_gui;
    LuaStorage lua_storage;
    LuaTime lua_time;
    LuaNetwork lua_network;
};

#endif