#ifndef APP_WRAPPER_HPP
#define APP_WRAPPER_HPP

#include "path.hpp"
#include "json.hpp"
#include "filestream.hpp"

using json = nlohmann::json;

namespace app {
        
    template <typename T> 
    class AppWrapper {
        public:

            AppWrapper( const storage::Path& location )  {
                this->location = location;
                readManifest();
            }

            ~AppWrapper() {

            }

            void launch(void) {
                try {
                    instance = new T( location, manifest );
                    instance->start();
                    instance->quit();
                    delete instance;    
                } catch (const std::exception& e) {
                    // prise en charge des erreurs
                    return;
                }
            }   

        private:
        
            void readManifest(void) {

                /* get raw data from the manifest */
                storage::Path manifestPath = this->location / "manifest.json";
                if(manifestPath.isfile()) {
                    storage::FileStream manifestStream (manifestPath.str(), storage::READ);
                    std::string manifestRaw = manifestStream.read();
                    this->manifest = json::parse(manifestRaw);
                }
                
                /* get the data needed by APP_WRAPPER */
                this->name = manifest["name"].empty() ? "(untitled)" : manifest["name"];
                this->icon = manifest["icon"].empty() ? "icon.png" : manifest["name"];
                
            }

            std::string name;
            std::string icon;

            T* instance;
            storage::Path location;
            json manifest;
    };

}
#endif /* APP_WRAPPER_HPP */