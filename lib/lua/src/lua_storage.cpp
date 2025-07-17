#include "lua_storage.hpp"

#include "lua_file.hpp"

#include <libsystem.hpp>

LuaStorage::LuaStorage(LuaFile* lua)
{
    this->lua = lua;
}

bool LuaStorage::newDir(std::string path)
{
    return convertPath(path).newdir();
}

bool LuaStorage::renameFile(std::string oldpath, std::string newpath)
{
    return convertPath(oldpath).rename(convertPath(newpath));
}

bool LuaStorage::renameDir(std::string oldpath, std::string newpath)
{
    return convertPath(oldpath).rename(convertPath(newpath));
}

bool LuaStorage::deleteFile(std::string path)
{
    return convertPath(path).remove();
}

bool LuaStorage::deleteDir(std::string path)
{
    return convertPath(path).remove();
}

bool LuaStorage::isFile(std::string path)
{
    return convertPath(path).isfile();
}

bool LuaStorage::isDir(std::string path)
{
    return convertPath(path).isdir();
}

std::vector<std::string> LuaStorage::listDir(std::string path)
{
    return convertPath(path).listdir();
}

bool LuaStorage::legalPath(storage::Path path)
{
    if (!this->lua->perms.acces_files)
        return false;
    if (path.m_steps[0] == "/" && !this->lua->perms.acces_files_root)
        return false;

    return true;
}

storage::Path LuaStorage::convertPath(std::string path)
{
    // std::cout << "convertPath: " << path << std::endl;
    if (!legalPath(path))
        throw libsystem::exceptions::RuntimeError(
            "The app is not allowed to access this path: " + path
        );

    if (path[0] == '/')
    {
        // std::cout << "Returning path: " << path << std::endl;
        return path;
    }
    else
    {
        storage::Path fullPath = this->lua->directory / path;
        // std::cout << "Returning full path: " << this->lua->directory.str() << " + " << path << "
        // = " << fullPath.str() << std::endl;
        return fullPath;
    }
}

std::unique_ptr<LuaStorageFile> LuaStorage::file(std::string filename, int mode)
{
    storage::Path path(convertPath(filename));
    // std::cout << "path: " << path.str() << std::endl;

    return std::make_unique<LuaStorageFile>(path, mode);
}
