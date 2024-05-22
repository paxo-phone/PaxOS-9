#include "app.hpp"

#include <json.hpp>

namespace app
{
    std::vector<App> appList;

    bool request;
    AppRequest requestingApp;

    void init()
    {
        // Get all apps
        std::vector<std::string> dirs = storage::Path(APP_DIR).listdir();

        // Get already authorized apps
        storage::FileStream stream((storage::Path(PERMS_DIR) / "auth.list").str(), storage::READ);
        std::string allowedFiles = stream.read();
        stream.close();

        // List apps
        for (auto dir : dirs)
        {
            std::cout << (storage::Path(APP_DIR) / dir).str() << std::endl;

            const bool authorized = allowedFiles.find((storage::Path(APP_DIR) / dir).str()) != std::string::npos;
            storage::Path appConfigFile;

            // Check if app is already authorized
            if (authorized)
            {
                // If the app is already authorized
                // The app config is cached in the permission directory

                // NOTE : What's happening when updating the app's manifest ?

                appConfigFile = storage::Path(PERMS_DIR) / (dir + ".json");
            }
            else
            {
                // If the app is not already authorized
                // The app config is stored in the app's directory
                appConfigFile = storage::Path(APP_DIR) / dir / "manifest.json";
            }

            // Read the app's manifest
            storage::FileStream file2(appConfigFile.str(), storage::READ);
            std::string appConfigJSON = file2.read();
            file2.close();

            if (!nlohmann::json::accept(appConfigJSON))
            {
                std::cerr << "Invalid app config : " << (storage::Path(APP_DIR) / dir).str() << std::endl;
                continue;
            }

            nlohmann::json appConfig = nlohmann::json::parse(appConfigJSON);

            // Find display name
            // Default is application's dir name
            std::string appDisplayName = dir;

            if (appConfig.contains("display_name"))
            {
                appDisplayName = appConfig["display_name"];
            }

            // Push the app
            appList.push_back({
                dir,
                appDisplayName,
                storage::Path(APP_DIR) / dir  / "app.lua",
                appConfigFile,
                authorized
            });
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

        return {"", "", storage::Path(), storage::Path(), false};
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