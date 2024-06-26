#include "lua_gsm.hpp"
#include "gsm.hpp"
#include "conversation.hpp"

#ifdef ESP_PLATFORM
#include <Arduino.h>
#endif

namespace LuaGSM
{
    void newMessage(std::string number, std::string message)
    {
        GSM::newMessage(number, message);
    }

    void newCall(std::string number)
    {
#ifdef ESP_PLATFORM
        GSM::newCall(number);

        uint64_t timeout = millis() + 5000;
        while (GSM::state.callFailure == false && GSM::state.callState != GSM::CallState::CALLING && millis() < timeout)
        {
            StandbyMode::wait();
            std::cout << "Waiting for call" << std::endl;
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

    sol::table getMessages(const std::string &number, sol::state& lua)
    {
        Conversations::Conversation conv;
        conv.number = number;
        std::string convFilePath = std::string(MESSAGES_LOCATION) + "/" + number + ".json";
        Conversations::loadConversation(convFilePath, conv);
        
        sol::table messages = lua.create_table();
        for (const auto msg : conv.messages)
        {
            messages.add(msg);
        }
        return messages;
    }
}
