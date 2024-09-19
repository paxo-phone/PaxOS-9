#include "app.hpp"

#include <libsystem.hpp>
#include <standby.hpp>

namespace AppManager {
    App::App(const std::string& name, const storage::Path& path, const storage::Path& manifest, const bool auth)
        : name(name), fullName(name), path(path), manifest(manifest),
          auth(auth),
          luaInstance(nullptr), app_state(NOT_RUNNING), background(false) {

        std::cout << "App: \"" << name << "\" \"" << fullName << "\"" << std::endl;

        if (!manifest.exists()) {
            throw libsystem::exceptions::InvalidArgument("Attempting to create an app with a non-existing manifest: " + manifest.str() + ".");
        }
    }

    void App::run(const bool background, const std::vector<std::string>& parameters) {
        if (!auth) {
            throw libsystem::exceptions::RuntimeError("App is not authorized to run");
        }

        libsystem::log("Manifest: " + manifest.str());

        app_state = background ? RUNNING_BACKGROUND : RUNNING;

        auto manifestFileStream = storage::FileStream(manifest.str(), storage::READ);

        std::string manifestData = manifestFileStream.read();

        manifestFileStream.close();

        if (!nlohmann::json::accept(manifestData)) {
            throw libsystem::exceptions::RuntimeError("Invalid app manifest, incorrect JSON format.");
        }

        if (const nlohmann::basic_json<> manifestJson = nlohmann::json::parse(manifestData); manifestJson.contains("permissions")) {
            // PaxoLua app

            if (!manifestJson.contains("main") || !manifestJson["main"].is_string()) {
                throw libsystem::exceptions::RuntimeError("Invalid app manifest, missing 'main'.");
            }

            storage::Path appDirectory = path / "..";

            storage::Path appMain = appDirectory / manifestJson["main"].get<std::string>();

            if (!appMain.exists()) {
                throw libsystem::exceptions::RuntimeError("Invalid app main, missing file: " + appMain.str() + ".");
            }

            m_luaEnvironment = std::make_shared<paxolua::LuaEnvironment>(appDirectory);
            paxolua::helper::applyManifest(m_luaEnvironment.get(), manifest);
            m_luaEnvironment->loadFile(appMain);
            m_luaEnvironment->run();
        } else {
            // Legacy app

            luaInstance = std::make_shared<LuaFile>(path, manifest);
            luaInstance->app = this;

            luaInstance->load();
            luaInstance->run(parameters);
        }

        // Clear used JSON data
        manifestData.clear();

        // Add app to the stack
        appStack.push_back(this);
    }

    void App::wakeup() {
        if (app_state == NOT_RUNNING) {
            std::cerr << "Error: App is not running" << std::endl;
            return;
        }

        app_state = RUNNING;
        luaInstance->wakeup();
    }

    void App::sleep() {
        app_state = SLEEPING;
    }

    bool App::isRunning() const {
        return app_state == RUNNING || app_state == RUNNING_BACKGROUND;
    }

    bool App::isLoaded() const {
        return app_state != NOT_RUNNING;
    }

    bool App::isVisible() const {
        if (!isLoaded())
            return false;

        return appStack.back() == this;
    }

    void App::kill() {
        // PaxoLua
        if (m_luaEnvironment != nullptr) {
            m_luaEnvironment.reset();

            app_state = NOT_RUNNING;

            libsystem::log("App killed.");
        }

        // Legacy
        if (luaInstance != nullptr) {
            luaInstance->stop();
            luaInstance.reset(); // delete luaInstance

            app_state = NOT_RUNNING;

            std::cout << "App killed" << std::endl;
        }
    }

    void App::requestAuth() {
        Window win;

        auto *label = new Label(0, 0, 320, 400);

        storage::FileStream stream(manifest.str(), storage::READ);
        std::string data = stream.read();
        stream.close();

        label->setText(data);
        win.addChild(label);

        auto *btn = new Button(35, 420, 250, 38);
        btn->setText("Accepter");
        win.addChild(btn);

        // TODO: Add "Cancel" button

        while (true) {
            win.updateAll();

            if (btn->isTouched()) {
                storage::FileStream streamP((storage::Path(PERMS_DIR) / "auth.list").str(), storage::APPEND);
                streamP.write(path.str() + "\n");
                streamP.close();

                manifest = storage::Path(PERMS_DIR) / (name + ".json");
                auth = true;

                storage::FileStream newPermCopy(manifest.str(), storage::WRITE);
                newPermCopy.write(data);
                newPermCopy.close();
            }
        }
    }

    std::string App::toString() const {
        return "{name = " + name + ", fullName = " + fullName + ", path = " + path.str() + ", manifest = " + manifest.
               str() + ", auth = " + std::to_string(auth) + ", state = " + std::to_string(app_state) + "}";
    }

    std::shared_ptr<paxolua::LuaEnvironment> App::getLuaEnvironment() {
        return m_luaEnvironment;
    }

    std::mutex threadsync;

    std::vector<std::shared_ptr<App> > appList;

    /**
     * The stack is used only for foreground applications THAT HAVE a Window created.
     */
    std::vector<App *> appStack;

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    // ReSharper disable once CppDFAConstantFunctionResult
    int pushError(lua_State *L, sol::optional<const std::exception &> maybe_exception,
                  const sol::string_view description) {
        std::shared_ptr<App> erroredApp;

        for (auto &app: appList) {
            // Get the corresponding Lua app
            if (app->luaInstance->lua.lua_state() == L) {
                // Save app
                erroredApp = app;

                // Store errors
                app->errors = std::string(description);
                if (maybe_exception) {
                    app->errors += ": " + std::string(maybe_exception->what());
                }

                // Stop the app
                app->app_state = App::AppState::NOT_RUNNING;

                //return 0;
            }
        }

        // Show error GUI

        std::cerr << "The App " << erroredApp->name << " encountered an error:" << std::endl;
        std::cerr << erroredApp->errors << std::endl;

        Window win;

        const auto label = new Label(0, 0, 320, 400);

        label->setText("The App " + erroredApp->name + " encountered an error:\n" + erroredApp->errors);
        win.addChild(label);

        auto *btn = new Button(35, 420, 250, 38);
        btn->setText("Quitter");
        win.addChild(btn);

        // Wait for action before closing GUI
        while (!hardware::getHomeButton()) // TODO: asynchronize this
        {
            win.updateAll();
            if (btn->isTouched()) {
                return 0;
            }
        }

        return 0;
    }

    void askGui(const LuaFile* lua) {
        App* app = lua->app;

        if (lua->lua_gui.mainWindow == nullptr) {
            for (auto it = appStack.begin(); it != appStack.end(); ++it) {
                if (*it == app) {
                    app->app_state = App::AppState::NOT_RUNNING;
                    appStack.erase(it);
                    break;
                }
            }

            return;
        }

        // if (appStack.empty() || appStack.back() != app) {
        //     appStack.push_back(app);
        // }
    }

    void loadDir(const storage::Path& directory, bool root = false) {
        std::vector<std::string> dirs = storage::Path(directory).listdir();

        storage::FileStream stream((storage::Path(PERMS_DIR) / "auth.list").str(), storage::READ);
        std::string allowedFiles = stream.read();
        stream.close();

        libsystem::log("auth.list : " + allowedFiles);

        for (auto dir: dirs) {
            auto appPath = storage::Path(directory) / dir;
            libsystem::log("Loading app at \"" + appPath.str() + "\".");

            auto manifestPath = storage::Path(directory) / dir / "manifest.json";

            storage::FileStream manifestStream(manifestPath.str(), storage::READ);
            std::string manifestContent = manifestStream.read();
            manifestStream.close();

            if (!nlohmann::json::accept(manifestContent)) {
                std::cout << "Error: invalid manifest at \"" << manifestPath.str() << "\"" << std::endl;
                continue;
            }

            nlohmann::json manifest = nlohmann::json::parse(manifestContent);

            std::shared_ptr<App> app;

            if (root) {
                app = std::make_shared<App>(
                    dir,
                    directory / dir / "app.lua",
                    directory / dir / "manifest.json",
                    true
                );
            } else if (allowedFiles.find(appPath.str()) != std::string::npos) {
                app = std::make_shared<App>(
                    dir,
                    storage::Path(APP_DIR) / dir / "app.lua",
                    storage::Path(PERMS_DIR) / (dir + ".json"),
                    true
                );
            } else {
                app = std::make_shared<App>(
                    dir,
                    storage::Path(APP_DIR) / dir / "app.lua",
                    storage::Path(APP_DIR) / dir / "manifest.json",
                    false
                );
            }

            app->fullName = dir;

            if (!dir.empty() && dir[0] == '.') {
                app->visible = false;
            } else {
                app->visible = true;
            }

            if (manifest["name"].is_string()) {
                app->name = manifest["name"];
            }

            app->background = false; // TODO : Allow background running

            if (manifest["autorun"].is_boolean()) {
                if (manifest["autorun"]) {
                    //appList.back().app_state = App::AppState::RUNNING_BACKGROUND; need to allocate -> use the run function
                    //appList.back().luaInstance->runBackground();
                }
            }

            // Add app to list
            libsystem::log("Loaded app : " + app->toString() + ".");
            appList.push_back(app);
        }
    }

    void init() {
        loadDir(storage::Path(APP_DIR));
        loadDir(storage::Path(SYSTEM_APP_DIR), true);
    }

    void loop() {
        updateForeground();
        updateBackground();
    }

    void updateForeground() {
        threadsync.lock();

        // Run tick on every app
        for (const auto& app: appList) {
            if(app->background == false)    // app is not in background
            {
                if (app->isRunning()) { // app is running
                    if (app->getLuaEnvironment() != nullptr) {
                        // Update PaxoLua app
                        app->getLuaEnvironment()->update();
                    }

                    if (app->luaInstance != nullptr) {
                        // Update legacy app
                        app->luaInstance->loop();
                    }
                } else if (std::find(appStack.begin(), appStack.end(), app.get()) != appStack.end()) // if app is no longer in the stack (no gui is running) -> kill it
                {
                    app->kill();
                }
            }
        }

        // Update foreground app GUI
        if (!appStack.empty()) {
            const App* app = appStack.back();

            if (app->luaInstance != nullptr) {
                app->luaInstance->lua_gui.update();
            }
        }

        threadsync.unlock();
    }

    void updateBackground() {
        threadsync.lock();

        // Run tick on every app
        for (const auto& app: appList) {
            if(app->background == true)    // app is in background
            {
                if (app->isRunning()) { // app is running
                    app->luaInstance->loop();
                }
            }
        }

        threadsync.unlock();
    }

    void quitApp() {
        if (appStack.empty()) {
            throw libsystem::exceptions::RuntimeError("Cannot quit an app if no app is running.");
        }

        // Get the currently running app
        App* app = appStack.back();

        // Check if the app on the top is a foreground app
        // TODO : Go down the stack ?
        if (app->background) {
            throw libsystem::exceptions::RuntimeError("Cannot quit an backgroundn app.");
        }

        // Kill the app
        app->kill();

        // Remove app from stack
        appStack.pop_back();
    }

    bool isAnyVisibleApp() {
        return !appStack.empty();
    }

    std::shared_ptr<App> get(const std::string& appName) {
        for (const auto& app: AppManager::appList) {
            if (app->fullName == appName) {
                return app;
            }
        }

        throw libsystem::exceptions::RuntimeError("App not found: " + appName);
    }

    std::shared_ptr<App> get(const uint8_t index) {
        if (index < appList.size()) {
            return appList[index];
        }
        throw libsystem::exceptions::OutOfRange("App index out of range");
    }

    std::shared_ptr<App> get(const lua_State* L) {
        for (const auto& app: appList) {
            if (app->luaInstance != nullptr && app->luaInstance->lua.lua_state() == L) {
                return app;
            }
        }
        throw libsystem::exceptions::RuntimeError("App not found for given lua_State instance");
    }

    std::shared_ptr<App> get(sol::state* L) {
        const auto it = std::find_if(
            appList.begin(),
            appList.end(),
            [L](const std::shared_ptr<App>& app) {
                return &app->luaInstance->lua == L;
            }
        );

        if (it != appList.end()) {
            return *it;
        }

        throw libsystem::exceptions::RuntimeError("App not found for given sol::state instance");
    }

    App* get(const LuaFile* luaInstance) {
        return luaInstance->app;

        // const auto it = std::find_if(
        //     appList.begin(),
        //     appList.end(),
        //     [luaInstance](const std::shared_ptr<App>& app) {
        //         return app->luaInstance.get() == luaInstance;
        //     }
        // );
        //
        // if (it != appList.end()) {
        //     return **it; // Probably not safe
        // }
        //
        // throw libsystem::exceptions::RuntimeError("App not found for given LuaFile instance");
    }

    std::shared_ptr<App> get(storage::Path path) {
        const auto it = std::find_if(
            appList.begin(),
            appList.end(),
            [&path](const std::shared_ptr<App>& app) {
                return app->path == path;
            }
        );

        if (it != appList.end()) {
            return *it;
        }

        throw libsystem::exceptions::RuntimeError("App not found at path: " + path.str());
    }

    void event_oncall() {
        threadsync.lock();
        for (auto &app: appList) {
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_oncall();
        }
        threadsync.unlock();
    }

    void event_onlowbattery() {
        threadsync.lock();
        for (auto &app: appList) {
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_onlowbattery();
        }
        threadsync.unlock();
    }

    void event_oncharging() {
        threadsync.lock();
        for (auto &app: appList) {
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_oncharging();
        }
        threadsync.unlock();
    }

    void event_onmessage() {
        threadsync.lock();
        for (auto &app: appList) {
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_onmessage();
        }
        threadsync.unlock();
    }

    void event_onmessageerror() {
        threadsync.lock();
        for (auto &app: appList) {
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_onmessageerror();
        }
        threadsync.unlock();
    }
} // namespace AppManager
