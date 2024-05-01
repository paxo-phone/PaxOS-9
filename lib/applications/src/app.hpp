#ifndef APPS_HPP
#define APPS_HPP

#include <vector>
#include <path.hpp>
#include <filestream.hpp>
#include <lua_file.hpp>
#include <gui.hpp>
#include <hardware.hpp>

#define APP_DIR "/apps"
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

#include <launcher.hpp>

#endif