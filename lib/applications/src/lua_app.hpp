#ifndef LUA_APP 
#define LUA_APP 

#include "app.hpp"
#include "path.hpp"

namespace app {
    
    class LuaApp : public App {
        
        public:
            LuaApp( const storage::Path& location, 
                 const json& manifest );
            ~LuaApp();

            void start(void);
            void quit(void);

        private:
            storage::Path entryPoint;
    };

}

#endif /* LUA_APP */