#ifndef GSM_MODULE_HPP
#define GSM_MODULE_HPP

#ifdef ESP_PLATFORM
#define gsm Serial2
#endif

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

extern const char* daysOfWeek[7];
extern const char* daysOfMonth[12];

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

inline long long getCurrentTimestamp();

namespace Gsm
{
    // --- Request Structure (Internal Use) ---
    struct Request
    {
        std::string command;
        std::function<bool(const std::string& response)> callback;
        std::shared_ptr<Request> next = nullptr;
    };

    // --- Public Enums ---
    enum class CallState
    {
        IDLE,
        DIALING,
        RINGING,
        ACTIVE,
        UNKNOWN // Initial or error state
    };

    namespace ExternalEvents
    {
        extern std::function<void(void)> onIncommingCall;
        extern std::function<void(void)> onNewMessage;
        extern std::function<void(void)> onNewMessageError;
    } // namespace ExternalEvents

    // --- HTTP Request Structure ---
    enum class HttpResult
    {
        OK,
        TIMEOUT,
        BUSY,
        INIT_FAILED,
        DNS_ERROR,
        CONNECTION_FAILED,
        MODULE_ERROR,
        SERVER_ERROR,
        NOT_FOUND,
        READ_ERROR
    };

    enum class HttpMethod
    {
        GET,
        POST
    };

    struct HttpRequest
    {
        HttpMethod method = HttpMethod::GET;
        std::string url;
        std::map<std::string, std::string> headers;
        std::string body;

        std::function<void(int http_code)> on_response;
        std::function<void(const std::string_view& data)> on_data;
        std::function<void(HttpResult result)> on_complete;
    };

    enum class HttpState
    {
        IDLE,
        INITIALIZING,
        ACTION_IN_PROGRESS,
        READING, // Covers the entire block-reading loop
        TERMINATING
    };
    static HttpState currentHttpState = HttpState::IDLE;
    static std::unique_ptr<HttpRequest> currentHttpRequestDetails;
    static int httpBytesTotal = 0;
    static int httpBytesRead = 0;

    // Forward declarations for our new helper functions
    static void _completeHttpRequest(HttpResult result);
    static void _queueNextHttpRead();
    void httpRequest(HttpRequest request);

    // --- Public Function Declarations ---

    // Initialization and Core Loop
    void init();
    void reboot();         // Reboot the GSM module
    void run();            // The main processing loop (handles internal updates)
    void uploadSettings(); // For configuring the module (e.g., enable URCs)
    void loop();           // The function containing the main run loop and periodic tasks

    // --- Public State Accessors (Getters) ---

    // Returns {RSSI (0-31, 99=unknown), BER (0-7, 99=unknown)}
    std::pair<int, int> getNetworkQuality();
    bool isNetworkQualityValid(); // Check if quality has been successfully read at least once

    // Returns true if GPRS is attached
    bool isConnected();
    bool isConnectedStateValid(); // Check if connection state has been read

    // Returns true if flight mode (RF OFF) is active
    bool isFlightModeActive();
    bool isFlightModeStateValid(); // Check if flight mode state has been read

    // Returns voltage in millivolts (mV), or -1 if unknown
    int getVoltage();
    double getBatteryLevel();
    bool isVoltageValid(); // Check if voltage has been read

    // Returns true if SIM requires a PIN
    bool isPinRequired();
    bool isPinStatusValid(); // Check if PIN status has been read

    // Returns true if PDU mode (CMGF=0) is active
    bool isPduModeEnabled();
    bool isPduModeStateValid(); // Check if PDU mode has been read

    // Returns the current call state
    CallState getCallState();

    // Returns the last caller ID received via +CLIP URC
    std::string getLastIncomingNumber();

    // --- Public Action Functions ---
    // Optional callbacks can be provided to know if the command was *accepted* (OK/ERROR)

    // Set the SIM PIN
    void setPin(
        const std::string& pin, std::function<void(bool success)> completionCallback = nullptr
    );

    // Enable/disable flight mode (true = RF OFF)
    void setFlightMode(
        bool enableFlightMode, std::function<void(bool success)> completionCallback = nullptr
    );

    // Set SMS mode (true = PDU mode (CMGF=0))
    void setPduMode(bool enablePdu, std::function<void(bool success)> completionCallback = nullptr);

    // Send SMS in PDU format (assumes PDU mode is set)
    // Callback provides command success and message reference number (-1 on failure)
    void sendMessagePDU(
        const std::string& pdu, int length,
        std::function<void(bool success, int messageRef)> completionCallback = nullptr
    );
    void sendMySms(const std::string& recipient, const std::string& text);
    void checkForMessages();

    // --- PDU Encoding ---
    std::pair<std::string, int>
        encodePduSubmit(const std::string& recipientNumber, const std::string& message);

    // Initiate a voice call
    void call(
        const std::string& number, std::function<void(bool success)> completionCallback = nullptr
    );

    // Accept incoming call
    void acceptCall(std::function<void(bool success)> completionCallback = nullptr);

    // Reject incoming call or hang up active/dialing call
    void rejectCall(std::function<void(bool success)> completionCallback = nullptr);

    // --- Public Refresh Functions (Optional) ---
    // Explicitly request an update of specific states. The update happens
    // asynchronously via the run() loop. Use getters to see the result later.
    void refreshNetworkQuality();
    void refreshConnectionStatus();
    void refreshFlightModeStatus();
    void refreshVoltage();
    void refreshPinStatus();
    void refreshPduModeStatus();

    // Call state is primarily updated via URCs and call actions

    // --- Time Sub-Namespace ---
    namespace Time
    {
        // Request network time synchronization
        // Note: Timezone handling is basic; it stores the modem's reported time and offset.
        void syncNetworkTime();

        // Getters for the last known time (returns -1 if not valid)
        int getYear();
        int getMonth();
        int getDay();
        int getHour();
        int getMinute();
        int getSecond();
        int getTimezoneOffsetQuarterHours(); // The offset reported by modem (zz in +/-zz)
        bool isTimeValid(); // Check if time has been successfully read at least once
    } // namespace Time

} // namespace Gsm

#endif
