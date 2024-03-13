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
    
    LuaStorageFile* file(storage::Path filename, bool mode);
    bool newDir(storage::Path path);
    bool renameFile(storage::Path oldpath, storage::Path newpath);
    bool renameDir(storage::Path oldpath, storage::Path newpath);
    bool deleteFile(storage::Path path);
    bool deleteDir(storage::Path text);
    bool isDir(storage::Path text);
    bool isFile(storage::Path text);
    std::vector<std::string> listDir(storage::Path path);

    bool legalPath(storage::Path path);
    storage::Path convertPath(storage::Path);

    private:
    LuaFile* lua = nullptr;
};

#endif