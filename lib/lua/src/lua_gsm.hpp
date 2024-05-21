#ifndef GSM_LUA_MODULE
#define GSM_LUA_MODULE

#include <string>
#include <cstdint>

namespace LuaGSM
{
    void newMessage(std::string number, std::string message);
    void newCall(std::string number);
    void endCall();
    void acceptCall();
    void rejectCall();
    std::string getNumber();
    uint8_t getCallState();
};

#endif
