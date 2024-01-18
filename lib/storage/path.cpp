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

#define PATH_SEPARATOR '/'

#if defined(__linux__) || defined(__APPLE__)
    #define SYSTEM_PATH_SEPARATOR '/'
#elif defined(_WIN32) || defined(_WIN64)
    #define SYSTEM_PATH_SEPARATOR '\\'
#else
    #define SYSTEM_PATH_SEPARATOR '/'
#endif

#include "SD.h"

bool storage::init()
{
    bool result = SD_init();

    std::cout << "init sd card: " << std::to_string(result) << std::endl;

    return result;
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
        
        std::string o = "";
        for(uint16_t i = 0; i < m_steps.size(); i++) {
            o += m_steps[i];
            if(i != m_steps.size() - 1) 
                o += SYSTEM_PATH_SEPARATOR;
        }

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
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

            std::vector<std::string> list;
            for (const auto &entry : std::filesystem::directory_iterator(this->str())) {
                if (onlyDirs && !entry.is_directory())
                    continue;

                list.push_back(entry.path().filename().string());
            }
            
            return list;

        #endif
        return {};
    }

    bool Path::exists(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return std::filesystem::exists(this->str());
        #endif
        return false;
    }

    bool Path::isfile(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return std::filesystem::is_regular_file(this->str());
        #endif
        return false;
    }

    bool Path::isdir(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return std::filesystem::is_directory(this->str());
        #endif
        return false;
    }

    bool Path::newdir(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

            if (!std::filesystem::exists(this->str()))
                return std::filesystem::create_directory(this->str());
            return false;
        
        #endif

        return false;
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
        return false;
    }

    bool Path::remove(void) const {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            return std::filesystem::remove(this->str());
        #endif
        return false;
    }

    void Path::rename(const Path& to) 
    {
        #if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
            std::filesystem::rename(this->str(), to.str());
            this->assign(to);
        #endif
    }
}

    