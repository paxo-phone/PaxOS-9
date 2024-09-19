#ifndef APPS_HPP
#define APPS_HPP

#include <vector>
#include <path.hpp>
#include <filestream.hpp>
#include <lua_file.hpp>
#include <gui.hpp>
#include <hardware.hpp>
#include <LuaEnvironment.hpp>
#include <mutex>

#define APP_DIR "/apps"
#define SYSTEM_APP_DIR "/sys_apps"
#define PERMS_DIR "/system"


namespace AppManager
{
    struct Permissions
    {
        // It's better with english (access)

        bool acces_gui = false; // graphics
        bool acces_files = false;
        bool acces_files_root = false;  // files from /
        bool acces_hardware = false;    // hardware, ex: light, flash, vibrator...
        bool acces_time = false;        // time
        bool acces_web_paxo = false;    // web only on paxo.fr
        bool acces_web = false;         // on any url
        bool acces_gsm = false;         // messages, calls
    };

    class App
    {
    public:
        App(const std::string& name, const storage::Path& path, const storage::Path& manifest, bool auth);

        /**
         * @param background Run in background
         * @param parameters List of parameters to send to the lua run function of the app
         */
        void run(bool background, const std::vector<std::string> &parameters = {});

        /**
         * @brief Wake up the app (if it was sleeping)
         */
        void wakeup();

        /**
         * @brief Put the app to sleep = the app will still loaded, but it will have neither events nor code that run.
         *
         * @note If the app is not running, this function does nothing
         */
        void sleep();
        /**
         * @return true if the app is running (and not sleeping)
         */
        [[nodiscard]] bool isRunning() const;

        /**
         * @return true if the app is loaded and allocated (even if it's sleeping)
         */
        [[nodiscard]] bool isLoaded() const;

        /**
         * @return true if the app is both running and visible
         */
        [[nodiscard]] bool isVisible() const;

        /**
         * @brief Kill the app (if it was running)
         *
         * @note If the app is not running, this function does nothing
         */
        void kill();

        /**
         * @brief Request auth for the app, so it's manifest is agreed
         */
        void requestAuth();

        /**
         * @brief Get the lua environment of the app.
         *
         * @return A shared pointer to the lua environment.
         */
        std::shared_ptr<paxolua::LuaEnvironment> getLuaEnvironment();

        [[nodiscard]] std::string toString() const;

        std::string name;       // app name
        std::string fullName;   // app directory name, full name
        storage::Path path;     // app directory
        storage::Path manifest; // app manifest (can be in the app folder is not validated, or in the system folder if validated)
        bool auth;              // is allowed to run
        bool visible = false;   // is visible on the menu (if it has a . before the folder name)

        std::string errors;     // errors pushed from the app

        enum AppState {         // app state
            RUNNING,
            RUNNING_BACKGROUND,
            SLEEPING,
            NOT_RUNNING
        };

        std::shared_ptr<LuaFile> luaInstance;     // lua environment for the app

        uint8_t app_state;      // app state
        bool background;        // app is in background

    private:
        // New PaxoLua environment
        std::shared_ptr<paxolua::LuaEnvironment> m_luaEnvironment;
    };

    // TODO : Check if "extern" is needed

    extern std::mutex threadsync; // mutex for thread-safe operations between threads

    extern std::vector<std::shared_ptr<App>> appList;   // list of apps in the apps folder
    extern std::vector<App*> appStack;                 // stack of the apps that are using the GUI, the last one is shown on the screen

    int pushError(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description);
    void askGui(const LuaFile* lua);
    bool isAnyVisibleApp();

    void init();

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