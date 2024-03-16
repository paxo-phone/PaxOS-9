#include "lua_gsm.hpp"

#include "gsm.hpp"

namespace LuaGSM
{
    void newMessage(std::string number, std::string message)
    {
        GSM::newMessage(number, message);
    }

    void newCall(std::string number)
    {
        GSM::newCall(number);
    }

    std::string getNumber()
    {
        return GSM::state.callingNumber;
    }

    uint8_t getCallState()
    {
        return GSM::state.callState;
    }
};