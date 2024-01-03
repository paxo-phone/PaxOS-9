#include <string>
#include <fstream>
#include <algorithm>
#include <unordered_map>

#include "i18n_translate.hpp"

#include <filesystem>

#include "i18n_config.hpp"

namespace i18n {

    std::unordered_map<std::string, std::string> gTranslations = {};
    
    inline void ltrim(std::string& str) {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char c) {
            return !std::isspace(c);
        }));
    }

    inline void rtrim(std::string& str) {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](char c) {
            return !std::isspace(c);
        }).base(), str.end());
    }

    inline void trim(std::string& str) {
        rtrim(str);
        ltrim(str);
    }

    void setTextDomain(const std::string& domain) {

        gTranslations.clear();

        std::string path = domain + "." + langToString(getLang()) + ".i18n";
        
        std::ifstream file (path);
        if( file.is_open() ) {
            
            bool inquotes = false;
            
            std::string key   = "";
            std::string temp  = "";
            char c;

            while( file.get(c) ) {
                
                if(c == '"') {
                    inquotes = !inquotes;
                    continue;
                }

                if(inquotes) { // ignore everything if it's between quotes
                    temp += c;
                    continue;
                }

                if(c == '=' && key.empty()) {
                    key = temp;
                    trim(key);
                    temp.clear();
                    continue;
                }

                if(c == '\n') {
                    trim(temp);
                    gTranslations[key] = temp;
                    temp.clear();
                    key.clear();
                    continue;
                }

                temp += c;                    
            }
            
            if( ! key.empty() ) {
                trim(temp);
                gTranslations[key] = temp;
            }

        }

        file.close();
    }

    std::string getText(const std::string& key) {
        return gTranslations[key];
    }


}