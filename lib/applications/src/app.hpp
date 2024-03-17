

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

    extern std::vector<App> appList;

    void init();
    bool askPerm(App &app);
    void runApp(storage::Path path);
};