#ifndef GSM_HPP
#define GSM_HPP

#include <functional>
#include <vector>
#include <string>
#include <threads.hpp>

extern const char *daysOfWeek[7];
extern const char *daysOfMonth[12];

#define f_Date(annee, mois) (((mois) <= 2) ? ((annee) - 1) : (annee))
#define g_Date(mois) (((mois) <= 2) ? ((mois) + 13) : ((mois) + 1))

struct Date
{
    int jour;
    int mois;
    long int annee;
};

#define myCalculOfDay(d) (1461 * f_Date(d.annee, d.mois) / 4 + 153 * g_Date(d.mois) / 5 + d.jour)
#define myWhatDay(d_) ((myCalculOfDay(d_) - 621049) % 7)

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
        std::function<void(void)> function;
        uint8_t priority;
    };

    struct Key
    {
        std::string key;
        std::function<void()> function;
    };

    struct State
    {
        CallState callState = NOT_CALLING;
        bool callFailure = false;
        std::string callingNumber = "";
    };

    struct Message
    {
        std::string number;
        std::string message;
        std::string date;
    };

    namespace ExternalEvents
    {
        extern std::function<void(void)> onIncommingCall;
        extern std::function<void(void)> onNewMessage;
        extern std::function<void(void)> onNewMessageError;
    }

    extern std::string data;
    extern std::vector<Request> requests;
    extern std::vector<Key> keys;
    // extern std::vector<Message> messages;        // received messages
    // extern std::vector<Message> pendingMessages; // messages pending
    extern std::vector<Message> messages;
    extern State state;
    extern uint16_t seconds, minutes, hours, days, months, years;
    extern float voltage;
    extern int networkQuality;

    void init();
    void reInit();
    void download(uint64_t timeout = 50);
    std::string send(const std::string &message, const std::string &answerKey, uint64_t timeout = 200);
    void process();
    void checkRequest();
    void run();

    void newMessage(std::string number, std::string message);
    void onMessage();

    void newCall(std::string number);
    void endCall();
    void acceptCall();
    void rejectCall();

    int getBatteryLevel();
    void getHour();

    int getNetworkStatus();

    std::string getCurrentTimestamp();
    std::string getCurrentTimestampNoSpaces();
    void clearFrom(const std::string &from, const std::string &to);
    void appendRequest(Request request);
}

#endif // GSM_HPP
