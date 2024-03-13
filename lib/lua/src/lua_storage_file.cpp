#include "lua_storage_file.hpp"

#include <iostream>

LuaStorageFile::LuaStorageFile(storage::Path filename, bool mode)
{
    std::cout << filename.str() << std::endl;
    this->filename = filename;
    this->mode = mode;
}

void LuaStorageFile::open()
{
    file.open(filename.str(), (mode)?(storage::WRITE):(storage::READ));
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
