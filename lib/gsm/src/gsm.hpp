#ifndef GSM_HPP
#define GSM_HPP

#include <functional>
#include <vector>
#include <string>
#include <cstdint>
#include <mutex>

#define BAUDRATE 921600

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

    // received data = RX buffer
    extern std::string data;
    // list of pending requests
    extern std::vector<Request> requests;
    // list of keys watched in the data received to run events
    extern std::vector<Key> keys;
    // module state: 
    extern State state;
    extern uint16_t seconds, minutes, hours, days, months, years;
    extern float voltage;
    extern int networkQuality;
    extern std::mutex coresync;

    // initialize the modem, power it on is required
    void init();
    // reinitialize the serial when the clock speed change to stay in sync
    void reInit();
    // download data from the modem, with a timeout
    void download(uint64_t timeout = 50);
    // send a request and return the answer
    std::string send(const std::string &message, const std::string &answerKey, uint64_t timeout = 200);
    // process the received data and run the related functions if needed (ex: calls, messages, ...)
    void process();
    // check if the data contains a request to be processed
    void checkRequest();
    // run the loop of the GSM thread
    void run();

    // Message related functions
    void newMessage(std::string number, std::string message);   // send message
    void onMessage();    // run the onMessage event -> save the messages

    // Call related functions
    void newCall(std::string number);    // send a call
    void endCall();                      // end a call
    void acceptCall();                   // accept a call
    void rejectCall();                   // reject a call

    // get the battery level from 0 to 1. TODO: moyenne sur plusieurs mesures
    double getBatteryLevel();            // get the battery level from 0 to 1. TODO: moyenne sur plusieurs mesures

    // Force to update the time
    void getHour();

    // return the network quality (0-31) and 99 if not available
    int getNetworkStatus();
    
    // return true if we are in flight mode
    bool isFlightMode();
    // set flight mode
    void setFlightMode(bool mode);

    // Network
    struct HttpHeader
    {
        enum Method
        {
            GET,
            POST
        };

        std::string url;
        Method httpMethod;
        std::string body;
    };

    class HttpRequest   // todo add a timeout if the callback is never called + add inner buffer for 2 cores requests
    {
        public:
        HttpRequest(HttpHeader header);
        ~HttpRequest();

        void send(std::function <void (uint8_t, uint64_t)> callback);    // return callback

        HttpHeader header;
        size_t readChunk(char* buffer);
        void close();
    
        enum RequestState
        {
            SETUP,      // the data is set up
            WAITING,    // waiting for the system to send the request
            SENT,       // the request has been sent, wait for the result
            RECEIVED,   // the request has been received, waiting for the callback to read
            END,
            ENDED       // the request has ended, need to be deleted
        };

        RequestState state = RequestState::SETUP;
        std::function <void (uint8_t, uint64_t)> callback;

        static std::vector<HttpRequest*> requests;
        static HttpRequest* currentRequest;
        static void manage();
        static void received();

        private:
        uint64_t dataSize = 0;
        uint64_t timeout = 0;   // date at which the request will timeout
        uint64_t readed = 0;

        void fastKill(uint8_t code = 400);
    };

    std::string getCurrentTimestamp();  // return the current timestamp formated
    std::string getCurrentTimestampNoSpaces();  // return the current timestamp formated without spaces
    void clearFrom(const std::string &from, const std::string &to);  // remove data between from and to: used to remove processed commands from the buffer
    void appendRequest(Request request);    // ask the GSM thread to run a request
}

#endif // GSM_HPP
