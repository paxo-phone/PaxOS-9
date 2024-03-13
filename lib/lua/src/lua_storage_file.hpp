#ifndef LUA_STORAGE_FILE_MODULE
#define LUA_STORAGE_FILE_MODULE

#include <path.hpp>
#include <filestream.hpp>

class LuaStorageFile
{
    public:
    LuaStorageFile(storage::Path filename, bool mode = false);

    void open();
    void close();
    void write(const std::string& text);
    char readChar();
    std::string readLine();
    std::string readAll();

    private:
    bool mode;  // false->read;   true->write;
    storage::Path filename;
    storage::FileStream file;
};

#endif