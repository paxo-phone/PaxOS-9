#ifndef GSM_HPP
#define GSM_HPP

#include <functional>
#include <vector>
#include <string>
#include <threads.hpp>

namespace GSM
{
    enum priority
    {
        high,
        normal,
        low
    };

    enum CallState
    {
        NOT_CALLING,
        RINGING,
        IN_CALL,
        CALLING
    };

    struct Request
    {
        std::function<void (void)> function;
        uint8_t priority;
    };

    struct Key
    {
        std::string key;
        std::function<void ()> function;
    };

    struct State
    {
        CallState callState = NOT_CALLING;
        std::string callingNumber = "";
    };

    struct Message
    {
        std::string number;
        std::string message;
    };

    extern std::string data;
    extern std::vector<Request> requests;
    extern std::vector<Key> keys;
    extern std::vector<Message> messages;   // received messages
    extern std::vector<Message> pendingMessages;    // messages pending
    extern State state;

    void init();
    void download(uint64_t timeout = 50);
    std::string send(const std::string &message, const std::string &answerKey, uint64_t timeout = 200);
    void process();
    void checkRequest();
    void run();

    void newMessage(std::string number, std::string message);
    void newCall(std::string number);
};

#endif
