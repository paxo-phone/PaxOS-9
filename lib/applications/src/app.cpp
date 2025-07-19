#include "app.hpp"

#include <graphics.hpp>
#include <libsystem.hpp>
#include <standby.hpp>

namespace AppManager
{
    App::App(
        const std::string& name, const storage::Path& path, const storage::Path& manifest,
        const bool auth
    ) :
        name(name), fullName(name), path(path), manifest(manifest), auth(auth),
        luaInstance(nullptr), app_state(NOT_RUNNING), background(false)
    {
    }

    void App::run(const std::vector<std::string>& parameters)
    {
        if (!auth)
        {
            requestAuth();

            if (!auth)
                return;
        }

        app_state = background ? RUNNING_BACKGROUND : RUNNING;

        if (!background)
            appStack.push_back(this);

        luaInstance = std::make_shared<LuaFile>(path, manifest);
        luaInstance->app = this;

        luaInstance->load();
        luaInstance->run(parameters);
    }

    void App::wakeup()
    {
        if (app_state == NOT_RUNNING)
        {
            std::cerr << "Error: App is not running" << std::endl;
            return;
        }

        app_state = RUNNING;
        luaInstance->wakeup();
    }

    void App::sleep()
    {
        app_state = SLEEPING;
    }

    bool App::isRunning() const
    {
        return this->luaInstance != nullptr;
    }

    bool App::isLoaded() const
    {
        return app_state != NOT_RUNNING;
    }

    bool App::isVisible() const
    {
        if (!isLoaded())
            return false;

        return appStack.back() == this;
    }

    void App::kill()
    {
        if (luaInstance != nullptr)
        {
            luaInstance->stop();
            luaInstance.reset(); // delete luaInstance

            for (auto it = appStack.begin(); it != appStack.end();)
                if (*it == this)
                    it = appStack.erase(it);
                else
                    ++it;

            app_state = NOT_RUNNING;

            std::cout << "App killed" << std::endl;
        }
    }

    bool didRequestAuth = false;

    void App::requestAuth()
    {
        didRequestAuth = true; // will be turned off in the main loop
        Window win;

        auto* label = new Label(0, 0, 320, 400);

        storage::FileStream stream(manifest.str(), storage::READ);
        std::string data = stream.read();
        stream.close();

        label->setText(
            "Voulez-vous autoriser l'application " + fullName +
            " à accéder aux permissions suivantes:\n" + data
        );
        win.addChild(label);

        auto* btn = new Button(35, 420, 250, 38);
        btn->setText("Accepter");
        win.addChild(btn);

        // TODO: Add "Cancel" button

        while (true)
        {
            win.updateAll();
            eventHandlerApp.update();

            if (hardware::getHomeButton())
                break;

            if (btn->isTouched())
            {
                AppManager::addPermission(this);
                break;
            }
        }
    }

    std::string App::toString() const
    {
        return "{name = " + name + ", fullName = " + fullName + ", path = " + path.str() +
               ", manifest = " + manifest.str() + ", auth = " + std::to_string(auth) +
               ", state = " + std::to_string(app_state) + "}";
    }

    std::mutex threadsync;

    std::vector<std::shared_ptr<App>> appList;

    /**
     * The stack is used only for foreground applications THAT HAVE a Window created.
     */
    std::vector<App*> appStack;

    // ReSharper disable once CppParameterMayBeConstPtrOrRef
    // ReSharper disable once CppDFAConstantFunctionResult
    int pushError(
        lua_State* L, sol::optional<const std::exception&> maybe_exception,
        const sol::string_view description
    )
    {
        std::shared_ptr<App> erroredApp;

        for (auto& app : appList)
        {
            // Get the corresponding Lua app
            if (app->luaInstance->lua.lua_state() == L)
            {
                // Save app
                erroredApp = app;

                // Store errors
                app->errors = std::string(description);
                if (maybe_exception)
                    app->errors += ": " + std::string(maybe_exception->what());

                // Stop the app
                app->app_state = App::AppState::NOT_RUNNING;

                // return 0;
            }
        }

        // Show error GUI

        std::cerr << "The App " << erroredApp->name << " encountered an error:" << std::endl;
        std::cerr << erroredApp->errors << std::endl;

        Window win;

        const auto label = new Label(0, 0, 320, 400);

        label->setText(
            "The App " + erroredApp->name + " encountered an error:\n" + erroredApp->errors
        );
        win.addChild(label);

        auto* btn = new Button(35, 420, 250, 38);
        btn->setText("Quitter");
        win.addChild(btn);

        // Wait for action before closing GUI
        while (!hardware::getHomeButton()) // TODO: asynchronize this
        {
            win.updateAll();
            if (btn->isTouched())
                return 0;
        }

        return 0;
    }

    void addPermission(App* app)
    {
        app->auth = true;

        storage::FileStream stream((storage::Path(PERMS_DIR) / "auth.list").str(), storage::APPEND);
        stream.write(app->path.str() + "\n");
        stream.close();

        storage::FileStream oman(app->manifest.str(), storage::READ);
        std::string manifest = oman.read();
        oman.close();

        storage::FileStream nman(
            (storage::Path(PERMS_DIR) / (app->fullName + ".json")).str(),
            storage::WRITE
        );
        nman.write(manifest);
        nman.close();
    }

    void loadDir(const storage::Path& directory, bool root = false, std::string prefix = "")
    {
        std::vector<std::string> dirs = storage::Path(directory).listdir();

        storage::FileStream stream((storage::Path(PERMS_DIR) / "auth.list").str(), storage::READ);
        std::string allowedFiles = stream.read();
        stream.close();

        // libsystem::log("auth.list : " + allowedFiles);

        for (auto dir : dirs)
        {
            auto appPath = storage::Path(directory) / dir;
            // libsystem::log("Loading app at \"" + appPath.str() + "\".");

            auto manifestPath = storage::Path(directory) / dir / "manifest.json";

            storage::FileStream manifestStream(manifestPath.str(), storage::READ);
            std::string manifestContent = manifestStream.read();
            manifestStream.close();

            if (!nlohmann::json::accept(manifestContent))
            {
                std::cerr << "Error: invalid manifest at \"" << manifestPath.str() << "\""
                          << std::endl;
                continue;
            }

            nlohmann::json manifest = nlohmann::json::parse(manifestContent);

            std::shared_ptr<App> app;

            std::string fullname = prefix.size() ? (prefix + "." + dir) : (dir);
            libsystem::log("Loading app \"" + fullname + "\".");

            std::cout << "path: " << appPath.str() << std::endl;

            if (root)
            {
                app = std::make_shared<App>(
                    dir,
                    directory / dir / "app.lua",
                    directory / dir / "manifest.json",
                    true
                );
            }
            else if (allowedFiles.find((appPath / "app.lua").str() + "\n") != std::string::npos)
            {
                app = std::make_shared<App>(
                    dir,
                    storage::Path(directory) / dir / "app.lua",
                    storage::Path(PERMS_DIR) / (fullname + ".json"),
                    true
                );
            }
            else
            {
                app = std::make_shared<App>(
                    dir,
                    storage::Path(directory) / dir / "app.lua",
                    storage::Path(directory) / dir / "manifest.json",
                    false
                );
            }

            app->fullName = fullname;

            if (!dir.empty() && dir[0] == '.')
                app->visible = false;
            else
                app->visible = true;

            if (manifest["name"].is_string())
                app->name = manifest["name"];

            if (manifest["background"].is_boolean())
                app->background = manifest["background"];

            if (manifest["subdir"].is_string()) // todo, restrict only for subdirs
            {
                if ((app.get()->path / "../" / manifest["subdir"]).exists())
                    loadDir(app.get()->path / "../" / manifest["subdir"], root, app->fullName);
                else
                    std::cerr << "Error: subdir \""
                              << (app.get()->path / "../" / manifest["subdir"]).str()
                              << "\" does not exist" << std::endl;
            }

            appList.push_back(app);

            if (manifest["autorun"].is_boolean())
            {
                if (manifest["autorun"] && app->background)
                    app.get()->run();
            }
        }
    }

    void init()
    {
        loadDir(storage::Path(APP_DIR));
        loadDir(storage::Path(SYSTEM_APP_DIR), true);
    }

    void loop()
    {
        updateForeground();
        updateBackground();
    }

    void updateForeground()
    {
        // printf("lock\n");
        threadsync.lock();
        // printf("locked\n");

        // Run tick on every app
        for (const auto& app : appList)
        {
            if (app->background == false) // app is not in background
            {
                if (app->isRunning())
                    app->luaInstance->loop();
            }
        }

        // Update foreground app GUI
        if (!appStack.empty())
        {
            App* app = appStack.back();

            if (app->luaInstance != nullptr)
            {
                // printf("-- 1\n");
                if (app == Keyboard_manager::app)
                    Keyboard_manager::update();
                else
                    app->luaInstance->lua_gui.update();
                // printf("-- 2\n");
            }

            if (app->luaInstance->lua_gui.mainWindow == nullptr) // app has no GUI
                app->kill();
        }

        threadsync.unlock();
        // printf("-- 3");
    }

    void updateBackground()
    {
        threadsync.lock();

        // Run tick on every app
        for (const auto& app : appList)
        {
            if (app->background == true) // app is in background
            {
                if (app->isRunning())
                { // app is running
                    app->luaInstance->loop();
                }
            }
        }

        threadsync.unlock();
    }

    void quitApp()
    {
        if (appStack.empty())
            throw libsystem::exceptions::RuntimeError("Cannot quit an app if no app is running.");

        // Get the currently running app
        App* app = appStack.back();

        // Kill the app
        app->kill();
    }

    bool isAnyVisibleApp()
    {
        return !appStack.empty();
    }

    std::shared_ptr<App> get(const std::string& appName)
    {
        for (const auto& app : AppManager::appList)
            if (app->fullName == appName)
                return app;

        libsystem::log("App not found: " + appName);
        return nullptr;
    }

    std::shared_ptr<App> get(const uint8_t index)
    {
        if (index < appList.size())
            return appList[index];
        libsystem::log("App index out of range");
        return nullptr;
    }

    std::shared_ptr<App> get(const lua_State* L)
    {
        for (const auto& app : appList)
            if (app->luaInstance != nullptr && app->luaInstance->lua.lua_state() == L)
                return app;
        libsystem::log("App not found for given lua_State instance");
        return nullptr;
    }

    std::shared_ptr<App> get(sol::state* L)
    {
        const auto it = std::find_if(
            appList.begin(),
            appList.end(),
            [L](const std::shared_ptr<App>& app)
            {
                return &app->luaInstance->lua == L;
            }
        );

        if (it != appList.end())
            return *it;

        libsystem::log("App not found for given sol::state instance");
        return nullptr;
    }

    App* get(const LuaFile* luaInstance)
    {
        return luaInstance->app;
    }

    std::shared_ptr<App> get(storage::Path path)
    {
        const auto it = std::find_if(
            appList.begin(),
            appList.end(),
            [&path](const std::shared_ptr<App>& app)
            {
                return app->path == path;
            }
        );

        if (it != appList.end())
            return *it;

        libsystem::log("App not found at path: " + path.str());
        return nullptr;
    }

    void event_oncall()
    {
        threadsync.lock();
        for (auto& app : appList)
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_oncall();
        threadsync.unlock();
    }

    void event_onlowbattery()
    {
        threadsync.lock();
        for (auto& app : appList)
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_onlowbattery();
        threadsync.unlock();
    }

    void event_oncharging()
    {
        threadsync.lock();
        for (auto& app : appList)
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_oncharging();
        threadsync.unlock();
    }

    void event_onmessage()
    {
        threadsync.lock();
        for (auto& app : appList)
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_onmessage();
        threadsync.unlock();
    }

    void event_onmessageerror()
    {
        threadsync.lock();
        for (auto& app : appList)
            if (app->luaInstance != nullptr && app->isRunning())
                app->luaInstance->event_onmessageerror();
        threadsync.unlock();
    }

    namespace Keyboard_manager
    {
        App* app = nullptr;
        std::function<void(std::string)> callback;
        std::unique_ptr<Keyboard> keyboard;

        void open(
            App* app, const std::string& placeholder, const std::string& defaultText,
            std::function<void(std::string)> callback
        )
        {
            printf("---- 1\n");
            graphics::setScreenOrientation(graphics::LANDSCAPE);
            // printf("---- 2\n");
            Keyboard_manager::app = app;
            Keyboard_manager::callback = callback;
            keyboard = std::make_unique<Keyboard>(defaultText);
            // printf("---- 3\n");
            keyboard->setPlaceholder(placeholder);
            // printf("---- 4\n");
        }

        void update()
        {
            // printf("--- 1\n");
            if (keyboard != nullptr)
            {
                keyboard->updateAll();

                // printf("--- 2\n");

                if (keyboard->quitting() || hardware::getHomeButton())
                {
                    // printf("--- 3\n");
                    close();
                }

                // printf("--- 4\n");
            }

            // printf("s--- 5\n");
        }

        void close(bool runcallback)
        {
            if (keyboard)
            {
                std::string result = keyboard->getText();
                keyboard.reset();
                Keyboard_manager::app = nullptr;
                graphics::setScreenOrientation(graphics::PORTRAIT);

                if (runcallback && callback)
                    callback(result);
            }
        }
    } // namespace Keyboard_manager
} // namespace AppManager
