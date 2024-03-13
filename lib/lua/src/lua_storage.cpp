#include "lua_storage.hpp"

#include "lua_file.hpp"

LuaStorage::LuaStorage(LuaFile* lua)
{
    this->lua = lua;
}

bool LuaStorage::newDir(storage::Path path)
{
    return convertPath(path).newdir();
}

bool LuaStorage::renameFile(storage::Path oldpath, storage::Path newpath)
{
    return convertPath(oldpath).rename(convertPath(newpath));
}

bool LuaStorage::renameDir(storage::Path oldpath, storage::Path newpath)
{
    return convertPath(oldpath).rename(convertPath(newpath));
}

bool LuaStorage::deleteFile(storage::Path path)
{
    return convertPath(path).remove();
}

bool LuaStorage::deleteDir(storage::Path path)
{
    return convertPath(path).remove();
}

bool LuaStorage::isFile(storage::Path path)
{
    return convertPath(path).isfile();
}

bool LuaStorage::isDir(storage::Path path)
{
    return convertPath(path).isdir();
}


std::vector<std::string> LuaStorage::listDir(storage::Path path)
{
    return convertPath(path).listdir();
}

bool LuaStorage::legalPath(storage::Path path)
{
    if(!this->lua->perms.acces_files)
        return false;
    if(path.m_steps[0]=="/" && !this->lua->perms.acces_files_root)
        return false;
    
    return true;
}

storage::Path LuaStorage::convertPath(storage::Path path)
{
    if (!legalPath(path))
        return this->lua->directory;
    
    if(path.m_steps[0]=="/")
        return path;
    else
        return this->lua->directory / path;
}

LuaStorageFile* LuaStorage::file(storage::Path filename, bool mode)
{
    if (!legalPath(filename))
        return nullptr;

    if(filename.m_steps[0]=="/")
        return new LuaStorageFile(filename, mode);
    else
        return new LuaStorageFile(this->lua->directory / filename, mode);
}