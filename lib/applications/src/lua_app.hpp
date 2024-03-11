#ifndef LUA_APP_RUN 
#define LUA_APP_RUN 

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

#endif /* LUA_APP_RUN */