#include "lua_app.hpp"
#include "path.hpp"
#include "filestream.hpp"

namespace app {

    LuaApp::LuaApp( const storage::Path& location, 
                    const json& manifest ) : App(location, manifest)
    {
        if ( ! manifest["entryPoint"].empty() )  {
            this->entryPoint.assign(location / manifest["entryPoint"]);
        }
    }
        
    LuaApp::~LuaApp() {

    }

    void LuaApp::start(void) {
        // lancement du moteur lua
    }

    void LuaApp::quit(void) {
        // arrêt du moteur lua
    }

}