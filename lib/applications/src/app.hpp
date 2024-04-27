#ifndef APPS_HPP
#define APPS_HPP

#include <vector>
#include <path.hpp>
#include <filestream.hpp>
#include <lua_file.hpp>
#include <gui.hpp>
#include <hardware.hpp>
#include <mutex>
#include <../network/network.hpp>

#define APP_DIR "/apps"
#define SYSTEM_APP_DIR "/sys_apps"
#define PERMS_DIR "/system"


namespace app
{
    struct App
    {
        std::string name;
        storage::Path path;
        storage::Path manifest;
        bool auth;
    };

    struct AppRequest
    {
        App app;
        std::vector<std::string> parameters;
    };

    extern std::vector<App> appList;

    extern bool request;
    extern AppRequest requestingApp;

    void init();
    App getApp(std::string appName);
    bool askPerm(App &app);
    void runApp(storage::Path path);
    void runApp(AppRequest app);
};

namespace AppManager
{
    struct Permissions
    {
        bool acces_gui = false;
        bool acces_files = false;
        bool acces_files_root = false;
        bool acces_hardware = false;
        bool acces_time = false;
        bool acces_web_paxo = false;
        bool acces_web = false;
        bool acces_gsm = false;
    };

    class App
    {
        public:
            App(std::string name, storage::Path path, storage::Path manifest, bool auth);
            ~App();

            void run(bool background, std::vector<std::string> parameters = {});

            void wakeup();
            void sleep();

            bool isRunning();   // app is active
            bool isLoaded();    // app is loaded and allocated
            bool isVisible();  // app is visible
            void kill();

            std::string name;
            std::string fullName;
            storage::Path path; // app directory
            storage::Path manifest;
            bool auth;  // is allowed to run
            bool visible = false;

            std::string errors;

            enum AppState
            {
                RUNNING,
                RUNNING_BACKGROUND,
                SLEEPING,
                NOT_RUNNING
            };

            LuaFile* luaInstance;
            uint8_t app_state;
            bool background;
    };

    extern std::mutex threadsync; // mutex for thread-safe operations between threads

    extern std::vector<App> appList;
    extern std::vector<App*> appStack;

    int pushError(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description);
    void askGui(LuaFile* lua);
    bool isAnyVisibleApp();

    void init();
    void loop();

    void event_oncall();
    void event_onlowbattery();
    void event_oncharging();
    void event_onmessage();
    void event_onmessageerror();

    App& get(std::string appName);
    App& get(uint8_t index);
    App& get(LuaFile* luaInstance);
    App& get(lua_State* L);
    App& get(sol::state* L);
    App& get(storage::Path path);
};

#include <launcher.hpp>

#endif
