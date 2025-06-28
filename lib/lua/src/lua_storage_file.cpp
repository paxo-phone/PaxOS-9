#include "lua_storage_file.hpp"

#include <iostream>

LuaStorageFile::LuaStorageFile(storage::Path filename, uint8_t mode)
{
    this->filename = filename;
    this->mode = mode;
}

void LuaStorageFile::open()
{
    if (mode == 0)
        file.open(filename.str(), storage::READ);
    else if (mode == 1)
        file.open(filename.str(), storage::WRITE);
    else if (mode == 2)
        file.open(filename.str(), storage::APPEND);
}

void LuaStorageFile::close()
{
    file.close();
}

void LuaStorageFile::write(const std::string& text)
{
    file.write(text);
}

char LuaStorageFile::readChar()
{
    return file.readchar();
}

std::string LuaStorageFile::readLine()
{
    return file.readline();
}

std::string LuaStorageFile::readAll()
{
    return file.read();
}
