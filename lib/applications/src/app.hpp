#ifndef APP_HPP
#define APP_HPP

#include "path.hpp"
#include "json.hpp"

using json = nlohmann::json;

namespace app {
    
    class App {

        public:

            App( const storage::Path& location, 
                 const json& manifest ) 
            {}
            
            ~App() {}

            virtual void start(void) = 0;
            virtual void quit(void) {}

        private:

    };

}

#endif /* APP_HPP */