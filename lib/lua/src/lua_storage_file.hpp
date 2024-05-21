#ifndef LUA_STORAGE_FILE_MODULE
#define LUA_STORAGE_FILE_MODULE

#include <path.hpp>
#include <filestream.hpp>
#include <SOL2/sol.hpp>

class LuaStorageFile
{
    public:
    LuaStorageFile(storage::Path filename, uint8_t mode = 0);
    ~LuaStorageFile(){std::cout << "LuaStorageFile destruct" << std::endl;}

    void open();
    void close();
    void write(const std::string& text);
    char readChar();
    std::string readLine();
    std::string readAll();

    private:
    uint8_t mode;  // false->read;   true->write;
    storage::Path filename;
    storage::FileStream file;
};

#endif