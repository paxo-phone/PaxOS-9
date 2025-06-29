#ifndef GSM_LUA_MODULE
#define GSM_LUA_MODULE

#include "conversation.hpp"

#include <SOL2/sol.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace LuaGSM
{
    void newMessage(std::string number, std::string message);
    void newCall(std::string number);
    void endCall();
    void acceptCall();
    void rejectCall();
    bool isPinNeeded();
    void setPin(std::string pin);
    void setFlightMode(bool mode);
    std::string getNumber();
    uint8_t getCallState();
    sol::table getMessages(const std::string& number, sol::state& lua);
}; // namespace LuaGSM

#endif
