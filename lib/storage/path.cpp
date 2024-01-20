#include <string>
#include <cstdint>
#include <vector>

#include <stdlib.h>
#include <iostream>

#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
    #include <filesystem>
    #include <fstream>
#endif

#include "path.hpp"
#include "SD.hpp"


#define PATH_SEPARATOR '/'

#if defined(__linux__) || defined(__APPLE__)
    #define SYSTEM_PATH_SEPARATOR '/'
#elif defined(_WIN32) || defined(_WIN64)
    #define SYSTEM_PATH_SEPARATOR '\\'
#else
    #define SYSTEM_PATH_SEPARATOR '/'
#endif


bool storage::init()
{
    #ifdef ESP_PLATFORM

    for (int i = 0; i < 4; i++)
    {
        if(SD_init())
            return true;
    }

    // Show error message on the screen?

    return false;

    #endif

    return true;
}

namespace storage {

    Path::Path(void) {}

    Path::Path(const std::string& raw) {
        parse(raw);
    }

    void Path::join(const Path& other) {
        for(uint16_t i = 0; i < other.m_steps.size(); i++) {
            m_steps.push_back(other.m_steps[i]);
        }
        
        simplify();
    }
    
    std::string Path::str(void) const {
        
        std::cout << "tostring" << std::endl;
        std::string o = "";

        #ifdef ESP_PLATFORM // specific to esp32 file system
            o += MOUNT_POINT;
            o += "/";
        #else
            o += "";
        #endif

        for(uint16_t i = 0; i < m_steps.size(); i++) {
            o += m_steps[i];
            if(i != m_steps.size() - 1) 
                o += SYSTEM_PATH_SEPARATOR;
        }

        std::cout << "tostring" << std::endl;
        return o;
    }

    Path Path::operator/(const Path& other) const {
        Path o;
        
        for(uint16_t i = 0; i < m_steps.size(); i++)
            o.m_steps.push_back(m_steps[i]);

        for(uint16_t i = 0; i < other.m_steps.size(); i++)
            o.m_steps.push_back(other.m_steps[i]);

        o.simplify();

        return o;
    }

    Path& Path::operator/=(const Path& other) {
        for(uint16_t i = 0; i < other.m_steps.size(); i++) {
            m_steps.push_back(other.m_steps[i]);
        }

        simplify();
        return (*this);
    }

    Path& Path::operator=(const Path& other) {
        this->assign(other);
        return (*this);
    }

    void Path::assign(const Path& other) {
        m_steps = other.m_steps;
    }

    void Path::clear(void) {
        this->m_steps.clear();
    }

    void Path::simplify(void) {
        
        uint16_t pos = 0;
        while(pos < m_steps.size() - 1) {
            if( m_steps[pos+1] == "..") {
                m_steps.erase(m_steps.begin() + pos, 
                              m_steps.begin() + pos + 2);
            }
            else {
                pos++;
            }
        }
    }

    void Path::parse(const std::string& raw) {
        
        uint16_t pos = 0;
        std::string temp = "";
        while(pos < raw.size()) {
            if(raw[pos] == PATH_SEPARATOR) {
                if( !temp.empty() ) {
                    m_steps.push_back(temp);
                    temp.clear();
                }
            }
            else {
                temp += raw[pos];
            }
            pos++;
        }

        if( ! temp.empty() )
            m_steps.push_back(temp);

        simplify();
    }

    std::vector<std::string> Path::listdir(bool onlyDirs) const {
        std::cout << "2" << std::endl;
        std::vector<std::string> list;
            std::cout << "dir: " << std::endl;
        std::cout << "3" << std::endl;

        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            std::filesystem::path dirPath = this->str();

        std::cout << "4" << std::endl;

            if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
                std::cerr << "Error: The directory does not exist or is not a valid directory." << std::endl;
                return {};
            }

        std::cout << "5" << std::endl;

            for (const auto &entry : std::filesystem::directory_iterator(dirPath)) {
                if (onlyDirs && !entry.is_directory())
                    continue;

                list.push_back(entry.path().filename().string());
            }

        std::cout << "6" << std::endl;
        #endif
        #ifdef ESP_PLATFORM
            DIR* dir = opendir(this->str().c_str());
            
            if (dir != NULL)
            {
                struct dirent* entry;
                while ((entry = readdir(dir)) != NULL) {
                    list.push_back(entry->d_name);
                }

                closedir(dir);
            }
        #endif
    
        return list;
    }

    bool Path::exists(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return std::filesystem::exists(this->str());
        #endif

        #ifdef ESP_PLATFORM
            struct stat st;
            if (stat(this->str().c_str(), &st) == 0)
                return true;

            return false;
        #endif
    }

    bool Path::isfile(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return std::filesystem::is_regular_file(this->str());
        #endif

        #ifdef ESP_PLATFORM
            struct stat st;
            if (stat(this->str().c_str(), &st) == 0) {
                return S_ISREG(st.st_mode);
            } else {
                return false;
            }
        #endif
    }

    bool Path::isdir(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return std::filesystem::is_directory(this->str());
        #endif

        #ifdef ESP_PLATFORM
            struct stat st;
            if (stat(this->str().c_str(), &st) == 0) {
                return S_ISDIR(st.st_mode);
            } else {
                return false;
            }
        #endif

        return false;
    }

    bool Path::newdir(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

            if (!std::filesystem::exists(this->str()))
                return std::filesystem::create_directory(this->str());
            return false;
        
        #endif

        #ifdef ESP_PLATFORM
            return mkdir(this->str().c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
        #endif
    }

    bool Path::newfile(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

            std::ofstream file(this->str());
            if (file.is_open()) {
                file.close();
                return true;
            }
            return false;
        #endif

        #ifdef ESP_PLATFORM
            FILE* file = fopen(this->str().c_str(), "w");
            if (file != NULL) {
                fclose(file);
                return true;
            } else {
                return false;
            }
        #endif
        
        return false;
    }

    bool Path::remove(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return std::filesystem::remove(this->str());
        #endif

        #ifdef ESP_PLATFORM
            return ::remove(this->str().c_str()) == 0;
        #endif
    }

    bool Path::rename(const Path& to) 
    {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            std::filesystem::rename(this->str(), to.str());
            this->assign(to);
            return true;
        #endif

        #ifdef ESP_PLATFORM
            return (::rename(this->str().c_str(), to.str().c_str()) == 0);
        #endif
    }
}

    