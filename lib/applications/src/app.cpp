#include "app.hpp"

namespace app
{
    std::vector<App> appList;

    bool request;
    AppRequest requestingApp;

    void init()
    {
        std::vector<std::string> dirs = storage::Path(APP_DIR).listdir();

        storage::FileStream stream((storage::Path(PERMS_DIR) / "auth.list").str(), storage::READ);
        std::string allowedFiles = stream.read();
        stream.close();

        for (auto dir : dirs)
        {
            std::cout << (storage::Path(APP_DIR) / dir).str() << std::endl;
            if(allowedFiles.find((storage::Path(APP_DIR) / dir).str()) != std::string::npos)
            {
                appList.push_back({dir, storage::Path(APP_DIR) / dir  / "app.lua", storage::Path(PERMS_DIR) / (dir + ".json"), true});
            }
            else
            {
                appList.push_back({dir, storage::Path(APP_DIR) / dir  / "app.lua", storage::Path(APP_DIR) / dir / "manifest.json", false});
            }
        }
    }

    App getApp(std::string appName)
    {
        for (auto &app : appList)
        {
            if(app.name == appName)
            {
                return app;
            }
        }

        return {"", storage::Path(), storage::Path(), false};
    }

    bool askPerm(App &app)
    {
        gui::elements::Window win;
        Label *label = new Label(0, 0, 320, 400);
        
        storage::FileStream stream(app.manifest.str(), storage::READ);
        std::string data = stream.read();
        stream.close();

        label->setText(data);
        win.addChild(label);
        
        Button *btn = new Button(35, 420, 250, 38);
        btn->setText("Accepter");
        win.addChild(btn);

        while (true)
        {
            win.updateAll();
            if(btn->isTouched())
            {
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

    void runApp(storage::Path path)
    {
        for (auto &app : appList)
        {
            if(app.path.str() == path.str())
            {
                if(app.auth)
                {
                    std::cout << "Succes: running app" << std::endl;
                    LuaFile luaApp(path, app.manifest);
                    luaApp.run();
                }
                else
                {
                    std::cout << "Asking for permissions" << std::endl;

                    if(askPerm(app))
                    {
                        LuaFile luaApp(path, app.manifest);
                        luaApp.run();
                    }
                }

                return;
            }
        }

        std::cout << "Error: no such app" << std::endl;
    }

    void runApp(AppRequest app)
    {
        LuaFile luaApp(app.app.path, app.app.manifest);
        luaApp.run(app.parameters);
    }
};

namespace AppManager {

    App::App(std::string name, storage::Path path, storage::Path manifest, bool auth)
        : name(std::move(name)), fullName(std::move(name)), path(std::move(path)), manifest(std::move(manifest)), auth(auth),
        luaInstance(nullptr), app_state(NOT_RUNNING), background(false)
        {
            std::cout << "App: \"" << name << "\" \"" << fullName << "\"" << std::endl;
        }

    App::~App() {
        if (luaInstance) {
            delete luaInstance;
        }
    }

    void App::run(bool background, std::vector<std::string> parameters) {
        if (!auth) {
            throw std::runtime_error("App is not authorized to run");
        }
        
        app_state = background ? RUNNING_BACKGROUND : RUNNING;

        luaInstance = new LuaFile(path, manifest);
        luaInstance->load();
        luaInstance->run(parameters);
    }

    void App::wakeup() {
        if(app_state == NOT_RUNNING)
        {
            std::cerr <<  "Error: App is not running" << std::endl;
            return;
        }

        app_state = RUNNING;
        luaInstance->wakeup();
    }

    void App::sleep()
    {
        app_state = SLEEPING;
    }

    bool App::isRunning() {
        return app_state == RUNNING || app_state == RUNNING_BACKGROUND;
    }

    bool App::isLoaded() {
        return app_state != NOT_RUNNING;
    }

    bool App::isVisible()
    {
        if(!isLoaded())
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

    std::mutex threadsync;

    std::vector<App> appList;
    std::vector<App*> appStack;

    int pushError(lua_State* L, sol::optional<const std::exception&> maybe_exception, sol::string_view description)
    {
        App* a = nullptr;

        for(auto& app : appList)
        {
            if(app.luaInstance->lua.lua_state() == L)
            {
                app.errors = std::string(description);
                if(maybe_exception)
                {
                    app.errors += ": " + std::string(maybe_exception->what());
                }

                a = &app;

                app.app_state = App::AppState::NOT_RUNNING;
                //return 0;
            }
        }

        std::cerr << "The App " << a->name << " encountered an error:" << std::endl;
        std::cerr << a->errors << std::endl;

        gui::elements::Window win;
        Label *label = new Label(0, 0, 320, 400);
        
        label->setText("The App " + a->name + " encountered an error:\n" + a->errors);
        win.addChild(label);
        
        Button *btn = new Button(35, 420, 250, 38);
        btn->setText("Quitter");
        win.addChild(btn);

        while (!hardware::getHomeButton())  // TODO: asynchronize this
        {
            win.updateAll();
            if(btn->isTouched())
            {
                return 0;
            }
        }

        return 0;
    }

    void askGui(LuaFile* lua)
    {
        App& app = AppManager::get(lua);

        if(lua->lua_gui.mainWindow == nullptr)
        {
            for (auto it = appStack.begin(); it != appStack.end(); ++it) {
                if (*it == &app) {
                    app.app_state = App::AppState::NOT_RUNNING;
                    appStack.erase(it);
                    break;
                }
            }

            return;
        }

        if(appStack.size() == 0 || appStack.back() != &app)
        {
            appStack.push_back(&app);
        }
    }

    void loadDir(storage::Path directory, bool root = false)
    {
        std::vector<std::string> dirs = storage::Path(directory).listdir();

        storage::FileStream stream((storage::Path(PERMS_DIR) / "auth.list").str(), storage::READ);
        std::string allowedFiles = stream.read();
        stream.close();

        for (auto dir : dirs)
        {
            std::cout << (storage::Path(directory) / dir).str() << std::endl;

            storage::FileStream manifestStream((storage::Path(directory) / dir / "manifest.json").str(), storage::READ);
            std::string manifestContent = manifestStream.read();
            manifestStream.close();

            if(!nlohmann::json::accept(manifestContent))
            {
                std::cout << "Error: invalid manifest" << std::endl;
                continue;
            }

            nlohmann::json manifest = nlohmann::json::parse(manifestContent);
            

            if(root)
            {
                appList.push_back({dir, directory / dir / "app.lua", directory / dir / "manifest.json", true});
            }
            else if(allowedFiles.find((storage::Path(directory) / dir).str()) != std::string::npos)
            {
                appList.push_back({dir, storage::Path(APP_DIR) / dir  / "app.lua", storage::Path(PERMS_DIR) / (dir + ".json"), true});
            }
            else
            {
                appList.push_back({dir, storage::Path(APP_DIR) / dir  / "app.lua", storage::Path(APP_DIR) / dir / "manifest.json", false});
            }

            appList.back().fullName = dir;

            if(dir.size() && dir[0] == '.')
                appList.back().visible = false;
            else
                appList.back().visible = true;
            
            if(manifest["name"].is_string())
                appList.back().name = manifest["name"];

            appList.back().background = false;

            if(manifest["autorun"].is_boolean())
            {
                if(manifest["autorun"])
                {
                    //appList.back().app_state = App::AppState::RUNNING_BACKGROUND; need to allocate -> use the run function
                    //appList.back().luaInstance->runBackground();
                }
            }
        }
    }

    void init()
    {
        loadDir(storage::Path(APP_DIR));
        loadDir(storage::Path(SYSTEM_APP_DIR), true);
    }

    void loop() {
        threadsync.lock();
        // Implementation for the main loop
        if(appStack.size() && hardware::getHomeButton())   // if the home button is pressed, remove the app from the stack, and kill it if it's not running in the background
        {
            while(hardware::getHomeButton());

            if(appStack.size())
            {
                if(appStack.back()->background == false)
                {
                    auto app = appStack.back();
                    int count = 0;
                    for (auto it = appStack.begin(); it != appStack.end(); ++it) {
                        if (*it == app) {
                            count++;
                        }
                    }

                    app->kill();
                }
                else
                {
                    std::cerr << "Error: app is in background" << std::endl;
                }

                appStack.pop_back();
            }
        }

        for (auto& app : appList) {
            if (app.isRunning()) {
                app.luaInstance->loop();
            }
            else if(app.luaInstance != nullptr)
            {
                app.kill();
            }
        }

        if(appStack.size())
        {
            appStack.back()->luaInstance->lua_gui.update();
        }

        threadsync.unlock();
    
        StandbyMode::wait();
    }

    bool isAnyVisibleApp()
    {
        return appStack.size();
    }

    App& get(std::string appName) {
        for (auto& app : AppManager::appList) {
            if(app.fullName == appName)
            {
                return app;
            }
        }

        throw std::runtime_error("App not found: " + appName);
    }

    App& get(uint8_t index) {
        if (index < appList.size()) {
            return appList[index];
        }
        throw std::out_of_range("App index out of range");
    }

    App& get(lua_State* L) {
        for (auto& app : appList)
        {
            if(app.luaInstance!=nullptr && app.luaInstance->lua.lua_state() == L)
            {
                return app;
            }
        }
        throw std::runtime_error("App not found for given lua_State instance");
    }

    App& get(sol::state* L) {
        auto it = std::find_if(appList.begin(), appList.end(),
                            [L](const App& app) { return &app.luaInstance->lua == L; });
        if (it != appList.end()) {
            return *it;
        }
        throw std::runtime_error("App not found for given sol::state instance");
    }

    App& get(LuaFile* luaInstance) {
        auto it = std::find_if(appList.begin(), appList.end(),
                            [luaInstance](const App& app) { return app.luaInstance == luaInstance; });
        if (it != appList.end()) {
            return *it;
        }
        throw std::runtime_error("App not found for given LuaFile instance");
    }

    App& get(storage::Path path) {
        auto it = std::find_if(appList.begin(), appList.end(),
                            [&path](const App& app) { return app.path == path; });
        if (it != appList.end()) {
            return *it;
        }
        throw std::runtime_error("App not found at path: " + path.str());
    }

    void event_oncall()
    {
        threadsync.lock();
        for (auto& app : appList)
        {
            if (app.luaInstance != nullptr && app.isRunning())
                app.luaInstance->event_oncall();
        }
        threadsync.unlock();
    }

    void event_onlowbattery() {
        threadsync.lock();
        for (auto& app : appList)
        {
            if (app.luaInstance != nullptr && app.isRunning())
                app.luaInstance->event_onlowbattery();
        }
        threadsync.unlock();
    }

    void event_oncharging() {
        threadsync.lock();
        for (auto& app : appList)
        {
            if (app.luaInstance != nullptr && app.isRunning())
                app.luaInstance->event_oncharging();
        }
        threadsync.unlock();
    }

    void event_onmessage() {
        threadsync.lock();
        for (auto& app : appList)
        {
            if (app.luaInstance != nullptr && app.isRunning())
                app.luaInstance->event_onmessage();
        }
        threadsync.unlock();
    }

    void event_onmessageerror()
    {
        threadsync.lock();
        for (auto& app : appList)
        {
            if (app.luaInstance != nullptr && app.isRunning())
                app.luaInstance->event_onmessageerror();
        }
        threadsync.unlock();
    }
} // namespace AppManager