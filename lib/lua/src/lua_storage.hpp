#ifndef LUA_STORAGE_MODULE
#define LUA_STORAGE_MODULE

#include <path.hpp>
#include <filestream.hpp>

#include "lua_storage_file.hpp"

class LuaFile;

class LuaStorage
{
    public:
    LuaStorage(LuaFile* lua);
    
    std::unique_ptr<LuaStorageFile> file(std::string filename, int mode);
    bool newDir(std::string path);
    bool renameFile(std::string oldpath, std::string newpath);
    bool renameDir(std::string oldpath, std::string newpath);
    bool deleteFile(std::string path);
    bool deleteDir(std::string text);
    bool isDir(std::string text);
    bool isFile(std::string text);
    std::vector<std::string> listDir(std::string path);

    bool legalPath(storage::Path path);
    storage::Path convertPath(storage::Path);

    private:
    LuaFile* lua = nullptr;
};

#endif