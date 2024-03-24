#include "lua_gsm.hpp"

#include "gsm.hpp"

#ifdef ESP_PLATFORM
#include <Arduino.h>
#endif

namespace LuaGSM
{
    void newMessage(std::string number, std::string message)
    {
        #ifdef ESP_PLATFORM
        GSM::newMessage(number, message);
        #endif
    }

    void newCall(std::string number)
    {
        #ifdef ESP_PLATFORM
        GSM::newCall(number);

        while (GSM::state.callFailure == false && GSM::state.callState != GSM::CallState::CALLING)
        {
            delay(10);
        }

        GSM::state.callFailure = false;
        #endif
    }

    void endCall()
    {
        #ifdef ESP_PLATFORM
        GSM::endCall();
        #endif
    }

    void acceptCall()
    {
        #ifdef ESP_PLATFORM
        GSM::acceptCall();
        #endif
    }

    void rejectCall()
    {
        #ifdef ESP_PLATFORM
        GSM::rejectCall();
        #endif
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