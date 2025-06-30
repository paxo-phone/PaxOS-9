#include "lua_gsm.hpp"

#include "conversation.hpp"

#include <clock.hpp>
#include <gsm2.hpp>
#include <standby.hpp>

#ifdef ESP_PLATFORM
#include <Arduino.h>
#endif

namespace LuaGSM
{
    void newMessage(std::string number, std::string message)
    {
        Gsm::sendMySms(number, message);
    }

    void newCall(std::string number)
    {
#ifdef ESP_PLATFORM
        static int callsuccess = 0; // 0 = not called, 1 = failed, 2 = call success

        Gsm::call(
            number,
            [&](bool success)
            {
                callsuccess = 1 + success;
            }
        );

        uint64_t timeout = os_millis() + 5000;
        while (callsuccess == 0 && os_millis() < timeout) StandbyMode::wait();
#endif
    }

    void endCall()
    {
#ifdef ESP_PLATFORM
        Gsm::rejectCall();
#endif
    }

    void acceptCall()
    {
#ifdef ESP_PLATFORM
        Gsm::acceptCall();
#endif
    }

    void rejectCall()
    {
#ifdef ESP_PLATFORM
        Gsm::rejectCall();
#endif
    }

    bool isPinNeeded()
    {
        return Gsm::isPinRequired();
    }

    void setPin(std::string pin)
    {
        Gsm::setPin(pin);
    }

    void setFlightMode(bool mode)
    {
        Gsm::setFlightMode(mode);
    }

    std::string getNumber()
    {
        return Gsm::getLastIncomingNumber();
    }

    uint8_t getCallState()
    {
        return (uint8_t) Gsm::getCallState();
    }

    sol::table getMessages(const std::string& number, sol::state& lua)
    {
        Conversations::Conversation conv;
        conv.number = number;
        std::string convFilePath = std::string(MESSAGES_LOCATION) + "/" + number + ".json";
        Conversations::loadConversation(convFilePath, conv);

        sol::table messages = lua.create_table();
        for (const auto msg : conv.messages) messages.add(msg);
        return messages;
    }
} // namespace LuaGSM
