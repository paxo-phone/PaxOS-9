#include "path.hpp"

#include <cstdint>
#include <filestream.hpp>
#include <iostream>
#include <libsystem.hpp>
#include <string>
#include <vector>

#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
#include <filesystem>
#include <fstream>
#else
#include <Arduino.h>
#include <SD.h>
#include <dirent.h>   // for Dir and Files
#include <sys/stat.h> // to check files
#endif

#define MOUNT_POINT "/sd"

#define PATH_SEPARATOR '/'

#if defined(__linux__) || defined(__APPLE__)
#define SYSTEM_PATH_SEPARATOR '/'
#elif defined(_WIN32) || defined(_WIN64)
#define SYSTEM_PATH_SEPARATOR '\\'
#else
#define SYSTEM_PATH_SEPARATOR '/'
#endif

#define VSPI_MOSI 23
#define VSPI_MISO 19
#define VSPI_SCLK 18
#define VSPI_CS 4

bool storage::init()
{
#ifdef ESP_PLATFORM

    constexpr uint8_t sdBeginTryCount = 4;

    delay(100);
    for (int i = 0; i < sdBeginTryCount; i++)
    {
        if (SD.begin(4, SPI, 5000000))
        {
            libsystem::log("SD card initialized.");
            return true;
        }

        delay(500);
        libsystem::log(
            "SD card initialization failed, try " + std::to_string(i + 1) + " of " +
            std::to_string(sdBeginTryCount) + "."
        );
    }

    // esp_restart();

    // Show error message on the screen?

    libsystem::log("SD card initialization failed.");

    return false;

#else

    return true;

#endif
}

namespace storage
{

    Path::Path(void) {}

    Path::Path(const std::string& raw)
    {
        parse(raw);
    }

    Path::Path(const Path& other)
    {
        this->assign(other);
    }

    void Path::join(const Path& other)
    {
        for (uint16_t i = 0; i < other.m_steps.size(); i++) m_steps.push_back(other.m_steps[i]);

        simplify();
    }

    void Path::join(const std::string& other)
    {
        this->join(Path(other));
    }

    std::string Path::str(void) const
    {
        std::string o = "";

#ifdef ESP_PLATFORM // specific to esp32 file system
        o += MOUNT_POINT;
        o += "/";
#else
        o += "./storage/";
#endif

        for (uint16_t i = 0; i < m_steps.size(); i++)
        {
            o += m_steps[i];
            if (i != m_steps.size() - 1)
                o += "/";
            // o += SYSTEM_PATH_SEPARATOR;
        }

        return o;
    }

    Path Path::operator/(const Path& other) const
    {
        Path o;

        for (uint16_t i = 0; i < m_steps.size(); i++) o.m_steps.push_back(m_steps[i]);

        for (uint16_t i = 0; i < other.m_steps.size(); i++) o.m_steps.push_back(other.m_steps[i]);

        o.simplify();

        return o;
    }

    Path Path::operator/(const std::string& other) const
    {
        return ((*this) / Path(other));
    }

    Path& Path::operator/=(const Path& other)
    {
        for (uint16_t i = 0; i < other.m_steps.size(); i++) m_steps.push_back(other.m_steps[i]);

        simplify();
        return (*this);
    }

    Path& Path::operator/=(const std::string& other)
    {
        return ((*this) /= Path(other));
    }

    Path& Path::operator=(const Path& other)
    {
        this->assign(other);
        return (*this);
    }

    Path& Path::operator=(const std::string& other)
    {
        return ((*this) = Path(other));
    }

    bool Path::operator==(const Path& other) const
    {
        return this->str() == other.str();
    }

    void Path::assign(const Path& other)
    {
        m_steps = other.m_steps;
    }

    void Path::assign(const std::string& other)
    {
        this->assign(Path(other));
    }

    void Path::clear(void)
    {
        this->m_steps.clear();
    }

    void Path::simplify(void)
    {
        std::vector<std::string> simplified_steps;
        for (const auto& step : m_steps)
        {
            if (step == "..")
            {
                if (!simplified_steps.empty() && simplified_steps.back() != "..")
                {
                    // Pop the last valid directory
                    simplified_steps.pop_back();
                }
                else
                {
                    // If no valid directory to go up, keep the ".."
                    simplified_steps.push_back(step);
                }
            }
            else if (step != ".")
            {
                simplified_steps.push_back(step);
            }
        }
        m_steps = std::move(simplified_steps);
    }

    void Path::parse(const std::string& raw)
    {

        uint16_t pos = 0;
        std::string temp = "";
        while (pos < raw.size())
        {
            if (raw[pos] == PATH_SEPARATOR)
            {
                if (!temp.empty())
                {
                    m_steps.push_back(temp);
                    temp.clear();
                }
            }
            else
            {
                temp += raw[pos];
            }
            pos++;
        }

        if (!temp.empty())
            m_steps.push_back(temp);

        simplify();
    }

    std::vector<std::string> Path::listdir(bool onlyDirs) const
    {
        std::vector<std::string> list;

#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
        std::filesystem::path dirPath = this->str();

        if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath))
        {
            std::cerr << "Error: The directory does not exist or is not a valid directory."
                      << std::endl;
            return {};
        }

        for (const auto& entry : std::filesystem::directory_iterator(dirPath))
        {
            if (onlyDirs && !entry.is_directory())
                continue;

            list.push_back(entry.path().filename().string());
        }

#endif
#ifdef ESP_PLATFORM
        DIR* dir = opendir(this->str().c_str());

        if (dir != NULL)
        {
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) list.push_back(entry->d_name);

            closedir(dir);
        }
#endif

        return list;
    }

    bool Path::exists(void) const
    {
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

    bool Path::isfile(void) const
    {
#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
        return std::filesystem::is_regular_file(this->str());
#endif

#ifdef ESP_PLATFORM
        struct stat st;
        if (stat(this->str().c_str(), &st) == 0)
            return S_ISREG(st.st_mode);
        else
            return false;
#endif
    }

    bool Path::isdir(void) const
    {
#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
        return std::filesystem::is_directory(this->str());
#endif

#ifdef ESP_PLATFORM
        struct stat st;
        if (stat(this->str().c_str(), &st) == 0)
            return S_ISDIR(st.st_mode);
        else
            return false;
#endif

        return false;
    }

    bool Path::newdir(void) const
    {
#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

        if (!std::filesystem::exists(this->str()))
            return std::filesystem::create_directory(this->str());
        return false;

#endif

#ifdef ESP_PLATFORM
        return mkdir(this->str().c_str(), S_IRWXU | S_IRWXG | S_IRWXO) == 0;
#endif
    }

    bool Path::copyTo(const Path& destinationPath) const
    {
#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

        if (std::filesystem::exists(this->str()))
        {
            /*
                enum class copy_options : unsigned short {
                    none = 0,
                    skip_existing = 1, overwrite_existing = 2, update_existing = 4,
                    recursive = 8,
                    copy_symlinks = 16, skip_symlinks = 32,
                    directories_only = 64, create_symlinks = 128, create_hard_links = 256
                };
            */
            if (std::filesystem::is_directory(this->str()))
                std::filesystem::copy(
                    this->str(),
                    destinationPath.str(),
                    std::filesystem::copy_options::recursive |
                        std::filesystem::copy_options::skip_existing
                );
            else
                std::filesystem::copy_file(
                    this->str(),
                    destinationPath.str(),
                    std::filesystem::copy_options::skip_existing
                );
            return true;
        }
        return false;

#endif

#ifdef ESP_PLATFORM
        if (destinationPath.exists())
            return false;

        if (this->isdir())
        {
            for (const std::string& entry : this->listdir())
            {
                Path newPath = destinationPath / entry;
                Path oldPath = *this / entry;

                if (!oldPath.copyTo(newPath))
                    return false;
            }
            return true;
        }
        else
        {
            destinationPath.newfile();

            storage::FileStream source(this->str(), storage::Mode::READ);
            storage::FileStream destination(destinationPath.str(), storage::Mode::WRITE);

            if (!source.isopen() || !destination.isopen())
            {
                source.close();
                destination.close();
                return false;
            }

            char nextChar = source.readchar();
            while (!nextChar)
            {
                std::string chunk = source.read(2047) + nextChar;
                destination.write(chunk);

                nextChar = source.readchar();
            }

            source.close();
            destination.close();

            return true;
        }
#endif
    }

    bool Path::newfile(void) const
    {
#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

        std::ofstream file(this->str());
        if (file.is_open())
        {
            file.close();
            return true;
        }
        return false;
#endif

#ifdef ESP_PLATFORM
        FILE* file = fopen(this->str().c_str(), "w");
        if (file != NULL)
        {
            fclose(file);
            return true;
        }
        else
        {
            return false;
        }
#endif

        return false;
    }

    bool Path::remove(void) const
    {
        std::cout << "Remove action" << std::endl;
        if (!this->exists())
            return false;
#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
        return std::filesystem::remove(this->str());
#endif

#ifdef ESP_PLATFORM
        if (this->isdir())
        {
            // serialcom::SerialManager::sharedInstance->commandLog("Remove dir");
            std::cout << "Remove dir" << std::endl;
            std::vector<std::string> children = this->listdir();
            if (!children.empty())
            {
                // serialcom::SerialManager::sharedInstance->commandLog("Remove children");
                std::cout << "Remove children" << std::endl;
                for (std::string child : children)
                {
                    Path childPath = *this / child;
                    if (!childPath.remove())
                        return false;
                    // serialcom::SerialManager::sharedInstance->commandLog("Removing child " +
                    // child);
                    std::cout << "Removing child " << childPath.str() << std::endl;
                }
            }
            return ::rmdir(this->str().c_str()) == 0;
        }
        else
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

    bool Path::copy(const Path& to)
    {
#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
        try
        {
            if (this->isfile())
            {
                // Copy single file
                std::filesystem::copy_file(
                    this->str(),
                    to.str(),
                    std::filesystem::copy_options::overwrite_existing
                );
            }
            else if (this->isdir())
            {
                // Copy directory and its contents recursively
                std::filesystem::copy(
                    this->str(),
                    to.str(),
                    std::filesystem::copy_options::recursive |
                        std::filesystem::copy_options::overwrite_existing
                );
            }
            return true;
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            std::cerr << "Copy failed: " << e.what() << std::endl;
            return false;
        }
#endif

#ifdef ESP_PLATFORM
        if (!this->exists())
            return false;

        if (this->isfile())
        {
            // Copy single file
            FILE* source = fopen(this->str().c_str(), "rb");
            if (!source)
                return false;

            FILE* dest = fopen(to.str().c_str(), "wb");
            if (!dest)
            {
                fclose(source);
                return false;
            }

            const size_t bufferSize = 1024;
            uint8_t buffer[bufferSize];
            size_t bytesRead;

            while ((bytesRead = fread(buffer, 1, bufferSize, source)) > 0)
            {
                if (fwrite(buffer, 1, bytesRead, dest) != bytesRead)
                {
                    fclose(source);
                    fclose(dest);
                    return false;
                }
            }

            fclose(source);
            fclose(dest);
            return true;
        }
        else if (this->isdir())
        {
            // Create destination directory
            if (!to.exists() && !to.newdir())
                return false;

            // Copy directory contents recursively
            std::vector<std::string> entries = this->listdir(false);
            bool success = true;

            for (const auto& entry : entries)
            {
                Path sourcePath = *this / entry;
                Path destPath = to / entry;

                if (sourcePath.isfile())
                {
                    success &= sourcePath.copy(destPath);
                }
                else if (sourcePath.isdir())
                {
                    if (!destPath.exists() && !destPath.newdir())
                    {
                        success = false;
                        break;
                    }
                    success &= sourcePath.copy(destPath);
                }
            }

            return success;
        }

        return false;
#endif

        return false;
    }
} // namespace storage
