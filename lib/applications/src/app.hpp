#ifndef APPS_HPP
#define APPS_HPP

#include <vector>
#include <path.hpp>
#include <filestream.hpp>
#include <lua_file.hpp>
#include <gui.hpp>
#include <hardware.hpp>
#include <mutex>

#define APP_DIR "/apps"
#define SYSTEM_APP_DIR "/sys_apps"
#define PERMS_DIR "/system"


// namespace app
// {
//     struct App
//     {
//         std::string name;
//         storage::Path path;
//         storage::Path manifest;
//         bool auth;
//     };
//
//     struct AppRequest
//     {
//         App app;
//         std::vector<std::string> parameters;
//     };
//
//     extern std::vector<App> appList;
//
//     extern bool request;
//     extern AppRequest requestingApp;
//
//     void init();
//     App getApp(const std::string& appName);
//     bool askPerm(App &app);
//     void runApp(const storage::Path& path);
//     void runApp(const AppRequest& app);
// };

namespace AppManager
{
    struct Permissions
    {
        // It's better with english (access)

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
        App(const std::string& name, const storage::Path& path, const storage::Path& manifest, bool auth);

        void run(bool background, const std::vector<std::string> &parameters = {});

        void wakeup();

        void sleep();

        [[nodiscard]] bool isRunning() const; // app is active
        [[nodiscard]] bool isLoaded() const; // app is loaded and allocated
        [[nodiscard]] bool isVisible() const; // app is visible
        void kill();

        void requestAuth();

        [[nodiscard]] std::string toString() const;

        std::string name;
        std::string fullName;
        storage::Path path; // app directory
        storage::Path manifest;
        bool auth; // is allowed to run
        bool visible = false;

        std::string errors;

        enum AppState {
            RUNNING,
            RUNNING_BACKGROUND,
            SLEEPING,
            NOT_RUNNING
        };

        std::shared_ptr<LuaFile> luaInstance;
        uint8_t app_state;
        bool background;
    };

    // TODO : Check if "extern" is needed

    extern std::mutex threadsync; // mutex for thread-safe operations between threads

    extern std::vector<std::shared_ptr<App>> appList;
    extern std::vector<App*> appStack;

    int pushError(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description);
    void askGui(const LuaFile* lua);
    bool isAnyVisibleApp();

    void init();

    /**
     * @deprecated
     */
    void loop();

    /**
     * Update every application.
     */
    void updateForeground();    // easy to understand
    void updateBackground();

    /**
     * Quit the currently foreground running application.
     */
    void quitApp();

    void event_oncall();
    void event_onlowbattery();
    void event_oncharging();
    void event_onmessage();
    void event_onmessageerror();

    std::shared_ptr<App> get(const std::string& appName);
    std::shared_ptr<App> get(uint8_t index);
    App* get(const LuaFile* luaInstance); // DEPRECATED
    std::shared_ptr<App> get(const lua_State* L);
    std::shared_ptr<App> get(sol::state* L);
    std::shared_ptr<App> get(storage::Path path);
};

#include <launcher.hpp>

#endif