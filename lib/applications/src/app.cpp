#include "app.hpp"

#include <system.hpp>

namespace app {
    std::vector<App> appList;

    bool request;
    AppRequest requestingApp;

    void init() {
        std::vector<std::string> dirs = storage::Path(APP_DIR).listdir();

        storage::FileStream stream((storage::Path(PERMS_DIR) / "auth.list").str(), storage::READ);
        const std::string allowedFiles = stream.read();
        stream.close();

        for (const auto& dir: dirs) {
            std::cout << (storage::Path(APP_DIR) / dir).str() << std::endl;
            if (allowedFiles.find((storage::Path(APP_DIR) / dir).str()) != std::string::npos) {
                appList.push_back({
                    dir, storage::Path(APP_DIR) / dir / "app.lua", storage::Path(PERMS_DIR) / (dir + ".json"), true
                });
            } else {
                appList.push_back({
                    dir, storage::Path(APP_DIR) / dir / "app.lua", storage::Path(APP_DIR) / dir / "manifest.json", false
                });
            }
        }
    }

    App getApp(const std::string& appName) {
        for (auto &app: appList) {
            if (app.name == appName) {
                return app;
            }
        }

        return {"", storage::Path(), storage::Path(), false};
    }

    bool askPerm(App &app) {
        gui::elements::Window win;
        auto *label = new Label(0, 0, 320, 400);

        storage::FileStream stream(app.manifest.str(), storage::READ);
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
                streamP.write(app.path.str() + "\n");
                streamP.close();

                app.manifest = storage::Path(PERMS_DIR) / (app.name + ".json");
                app.auth = true;

                storage::FileStream newPermCopy(app.manifest.str(), storage::WRITE);
                newPermCopy.write(data);
                newPermCopy.close();

                return true;
            }
        }
    }

    void runApp(const storage::Path& path) {
        for (auto &app: appList) {
            if (app.path.str() == path.str()) {
                if (app.auth) {
                    std::cout << "Succes: running app" << std::endl;
                    LuaFile luaApp(path, app.manifest);
                    luaApp.run();
                } else {
                    std::cout << "Asking for permissions" << std::endl;

                    if (askPerm(app)) {
                        LuaFile luaApp(path, app.manifest);
                        luaApp.run();
                    }
                }

                return;
            }
        }

        std::cout << "Error: no such app" << std::endl;
    }

    void runApp(const AppRequest& app) {
        LuaFile luaApp(app.app.path, app.app.manifest);
        luaApp.run(app.parameters);
    }
};

namespace AppManager {
    App::App(std::string name, const storage::Path& path, const storage::Path& manifest, const bool auth)
        : name(std::move(name)), fullName(std::move(name)), path(path), manifest(manifest),
          auth(auth),
          luaInstance(nullptr), app_state(NOT_RUNNING), background(false) {
        std::cout << "App: \"" << name << "\" \"" << fullName << "\"" << std::endl;
    }

    App::~App() {
        std::cout << "Unloading app: " << fullName << std::endl;

        delete luaInstance;
    }

    void App::run(const bool background, const std::vector<std::string>& parameters) {
        if (!auth) {
            throw libsystem::exceptions::RuntimeError("App is not authorized to run");
        }

        app_state = background ? RUNNING_BACKGROUND : RUNNING;

        luaInstance = new LuaFile(path, manifest);
        luaInstance->load();
        luaInstance->run(parameters);
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

    bool App::isVisible() {
        if (!isLoaded())
            return false;

        return appStack.back() == this;
    }

    void App::kill() {
        if (luaInstance != nullptr) {
            luaInstance->stop();
            delete luaInstance;
            luaInstance = nullptr;

            std::cout << "App killed" << std::endl;

            app_state = NOT_RUNNING;
        }
    }

    std::string App::toString() const {
        return "{name = " + name + ", fullName = " + fullName + ", path = " + path.str() + ", manifest = " + manifest.
               str() + ", auth = " + std::to_string(auth) + "}";
    }

    std::mutex threadsync;

    std::vector<std::shared_ptr<App> > appList;
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

    void askGui(LuaFile *lua) {
        App &app = AppManager::get(lua);

        if (lua->lua_gui.mainWindow == nullptr) {
            for (auto it = appStack.begin(); it != appStack.end(); ++it) {
                if (*it == &app) {
                    app.app_state = App::AppState::NOT_RUNNING;
                    appStack.erase(it);
                    break;
                }
            }

            return;
        }

        if (appStack.empty() || appStack.back() != &app) {
            appStack.push_back(&app);
        }
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
        threadsync.lock();
        // Implementation for the main loop
        if (!appStack.empty() && hardware::getHomeButton())
        // if the home button is pressed, remove the app from the stack, and kill it if it's not running in the background
        {
            while (hardware::getHomeButton()) {}

            if (!appStack.empty()) {
                if (appStack.back()->background == false) {
                    const auto app = appStack.back();
                    int count = 0; // TODO: Use ?

                    for (const auto& it : appStack) {
                        if (it == app) {
                            count++;
                        }
                    }

                    app->kill();
                } else {
                    std::cerr << "Error: app is in background" << std::endl;
                }

                appStack.pop_back();
            }
        }

        for (const auto& app: appList) {
            if (app->isRunning()) {
                app->luaInstance->loop();
            } else if (app->luaInstance != nullptr) {
                app->kill();
            }
        }

        if (!appStack.empty()) {
            appStack.back()->luaInstance->lua_gui.update();
        }

        threadsync.unlock();

        StandbyMode::wait();
    }

    bool isAnyVisibleApp() {
        return !appStack.empty();
    }

    App& get(const std::string& appName) {
        for (const auto& app: AppManager::appList) {
            if (app->fullName == appName) {
                return *app; // Probably not safe
            }
        }

        throw libsystem::exceptions::RuntimeError("App not found: " + appName);
    }

    App& get(const uint8_t index) {
        if (index < appList.size()) {
            return *appList[index]; // Probably not safe
        }
        throw libsystem::exceptions::OutOfRange("App index out of range");
    }

    App& get(const lua_State *L) {
        for (const auto& app: appList) {
            if (app->luaInstance != nullptr && app->luaInstance->lua.lua_state() == L) {
                return *app; // Probably not safe
            }
        }
        throw libsystem::exceptions::RuntimeError("App not found for given lua_State instance");
    }

    App& get(sol::state *L) {
        const auto it = std::find_if(
            appList.begin(),
            appList.end(),
            [L](const std::shared_ptr<App> &app) {
                return &app->luaInstance->lua == L;
            }
        );

        if (it != appList.end()) {
            return **it; // Probably not safe
        }

        throw libsystem::exceptions::RuntimeError("App not found for given sol::state instance");
    }

    App& get(LuaFile *luaInstance) {
        const auto it = std::find_if(
            appList.begin(),
            appList.end(),
            [luaInstance](const std::shared_ptr<App> &app) {
                return app->luaInstance == luaInstance;
            }
        );

        if (it != appList.end()) {
            return **it; // Probably not safe
        }

        throw libsystem::exceptions::RuntimeError("App not found for given LuaFile instance");
    }

    App& get(storage::Path path) {
        const auto it = std::find_if(
            appList.begin(),
            appList.end(),
            [&path](const std::shared_ptr<App> &app) {
                return app->path == path;
            }
        );

        if (it != appList.end()) {
            return **it; // Probably not safe
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
