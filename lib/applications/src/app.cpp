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
                    luaApp.run({"test et test"});
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
        luaApp.run();
    }
};