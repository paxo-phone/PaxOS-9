#include "gsm.hpp"
#include <delay.hpp>
#include "../../tasks/src/threads.hpp"

const char *daysOfWeek[7] = { "Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi" };
const char *daysOfMonth[12] = { "Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"};

#ifdef ESP_PLATFORM
#include <Arduino.h>
#endif

#define gsm Serial2
#define RX 26
#define TX 27

namespace GSM
{
    std::string data;
    std::vector<Request> requests;
    std::vector<Key> keys;
    std::vector<Message> messages;
    State state;
    uint16_t seconds, minutes, hours, days, months, years = 0;
    float voltage = 0;

    namespace ExternalEvents
    {
        std::function<void (void)> onIncommingCall;
    }

    void init()
    {
#ifdef ESP_PLATFORM
        pinMode(32, OUTPUT); // define pin mode

        digitalWrite(32, 1); // power on the module
        delay(1000);
        digitalWrite(32, 0);

        while (true)
        {
            gsm.begin(115200, SERIAL_8N1, RX, TX);

            while(gsm.available())
                gsm.read();
            
            gsm.println("AT\r");
            delay(1000);

            if (gsm.available())
            {
                std::cout << "Connected" << std::endl;
                return;
            }
            else
                std::cout << "Disconnected" << std::endl;
        }
#endif
    }

    void reInit()
    {
#ifdef ESP_PLATFORM
        gsm.begin(115200, SERIAL_8N1, RX, TX);
#endif
    }

    void download(uint64_t timeout)
    {
#ifdef ESP_PLATFORM
        uint64_t timer = 0;
        while (gsm.available() || timer + timeout > millis())
        {
            if(gsm.available())
            {
                timer = millis();
                data += gsm.read();
            }
        }
        /*if (timer != 0)
            std::cout << data << std::endl;*/
#endif
    }

    std::string send(const std::string &message, const std::string &answerKey, uint64_t timeout)
    {
#ifdef ESP_PLATFORM
        gsm.println((message + "\r").c_str());

        uint64_t lastChar = millis();
        std::string answer = "";

        while (lastChar + timeout > millis())       // save none related messages to data.
        {
            if (gsm.available())
            {
                answer += gsm.read();
                lastChar = millis();

                if(answer.find(answerKey) != std::string::npos)
                {
                    data += answer.substr(0, answer.find(answerKey) - 1);
                    break;
                }
            }
        }

        // true stream
        while (lastChar + timeout > millis() && answer.find("OK\13") == std::string::npos && answer.find("ERROR\13") == std::string::npos)
        {
            if (gsm.available())
            {
                answer += gsm.read();
                lastChar = millis();
            }
        }

        return answer;
#endif

        return "";
    }

    void appendRequest(Request request)
    {
        // ask the other core to add a request
        if(!request.function)
            std::cout << "request.function is invalid -> can't run the new request" << std::endl;
        else
            eventHandlerBack.setTimeout(new Callback<>(std::bind([](Request r){GSM::requests.push_back(r);}, request)), 0);
    }

    void process()
    {
#ifdef ESP_PLATFORM
        for (auto key : keys)
        {
            uint32_t i = data.find(key.key);

            if (i != std::string::npos)
            {
                key.function();
            }
        }
#endif
    }

    void checkRequest()
    {
        for (uint8_t pr = priority::high; pr <= priority::low; pr++) // for each priority
        {
            auto it = 0;
            while (it != requests.size())
            {
                if (requests[it].priority == pr)
                {
                    if(requests[it].function != nullptr)
                        requests[it].function();
                    requests[it].function = nullptr;
                }
                it++;
            }
        }

        requests.clear();
    }

    void clearFrom(const std::string& from, const std::string& to)
    {
        int first = data.find(from);
        if (first == std::string::npos) {
            return;
        }

        int end = data.find(to, first);
        if (end == std::string::npos) {
            return;
        }

        std::string before = data.substr(0, first);
        std::string after = "";
        if (end < data.length() - 1) {
            after = data.substr(end + to.length());
        }

        data = before + after;
    }


    void onRinging()
    {
        if(state.callState == CallState::RINGING)
            return;

        std::string defaultNumber = "unknown";

        int clccIndex = data.find("+CLCC:");
        if (clccIndex == std::string::npos) {
            state.callingNumber = defaultNumber;
        }

        int quoteIndex = data.find("\"", clccIndex);
        if (quoteIndex == std::string::npos) {
            state.callingNumber = defaultNumber;
        }

        int endQuoteIndex = data.find("\"", quoteIndex + 1);
        if (endQuoteIndex == std::string::npos) {
            state.callingNumber = defaultNumber;
        }

        state.callingNumber = data.substr(quoteIndex + 1, endQuoteIndex - quoteIndex - 1);
        state.callState = RINGING;

        clearFrom("RING", "\"\"");
        std::cout << "Number is calling: \"" << state.callingNumber << "\"" << std::endl;

        if(ExternalEvents::onIncommingCall)
            ExternalEvents::onIncommingCall();
    }

    void onHangOff()
    {
        state.callState = NOT_CALLING;

        clearFrom("VOICE CALL: END", "NO CARRIER");

        std::cout << "hanging off" << std::endl;
    }

    void onMessage()
    {
        
        std::cout << data << std::endl;
        clearFrom("+CMTI:", "\n");

        send("AT+CMGF=1", "AT+CMGF=1", 1000);

        std::string str = send("AT+CMGL=\"REC UNREAD\"", "AT+CMGL", 1000);

        for (int64_t i = 0; i < str.size();)
        {
            std::string number, message, date;
            int64_t j = str.find("+CMGL:", i);

            if (j == std::string::npos)
            {
                break;
            }

            int64_t k = str.find("\"", j);
            k = str.find("\"", k+1);
            k = str.find("\"", k+1);

            number = str.substr(k+1, str.find("\"", k+1)-k-1);

            k = str.find("\"", k+1);
            k = str.find("\"", k+1);
            k = str.find("\"", k+1);
            k = str.find("\"", k+1);

            date = str.substr(k+1, str.find("\"", k+1)-k-4);

            k = str.find("\"", k+1);

            message = str.substr(k+2+1, str.find(0x0D, k+1)-k);

            messages.push_back({number, message});
            std::cout << "messages: " << messages.size() << std::endl;

            i = j+1;
        }

        send("AT+CMGD=1,3", "AT+CMGD", 1000);
    }

    void sendMessage(const std::string &number, const std::string &message)
    {
        send("AT+CMGS=\"" + number + "\"\r", ">");
        send(message + char(26), "OK");
    }

    void newMessage(std::string number, std::string message)
    {
        appendRequest({std::bind(&GSM::sendMessage, number, message), priority::normal});
    }

    void sendCall(const std::string &number)
    {
        std::cout << "Calling " << number << std::endl;
        if(send("ATD" + number + ";", "OK", 2000).find("OK") != std::string::npos)
        {
            std::cout << "Call Success!" << std::endl;
            state.callState = CallState::CALLING;
            state.callingNumber = number;
        }
        else
        {
            std::cout << "Call Error!" << std::endl;
            state.callFailure = true;
        }
    }

    void newCall(std::string number)
    {
        std::cout << "new call " << number << std::endl;
        appendRequest({std::bind(&GSM::sendCall, number), priority::high});
    }

    void endCall()
    {
        appendRequest({[](){ GSM::send("AT+CHUP", "OK"); }, priority::high});
    }

    void acceptCall()
    {
        requests.push_back({[](){ GSM::send("ATA", "OK"); }, priority::high});
    }

    void rejectCall()
    {
        endCall();
    }

    void getVoltage()
    {
        std::string answer = send("AT+CBC", "OK");

        int start = answer.find("+CBC: ") + 6;
        int end = answer.find("V", start);

        if(start == std::string::npos || end == std::string::npos)  // maybe wrong
            return;

        std::string voltage_str = answer.substr(start, end - start);

        try
        {
            voltage = std::stof(voltage_str);
        }
        catch (std::exception)
        {
            return;
        }
    }

    int getBatteryLevel()
    {
        if(voltage>4.12)
            return 100;
        else if(voltage>4.03)
            return 95;
        else if(voltage>3.99)
            return 90;
        else if(voltage>3.94)
            return 85;
        else if(voltage>3.90)
            return 80;
        else if(voltage>3.86)
            return 75;
        else if(voltage>3.82)
            return 70;
        else if(voltage>3.77)
            return 65;
        else if(voltage>3.74)
            return 60;
        else if(voltage>3.70)
            return 55;
        else if(voltage>3.66)
            return 50;
        else if(voltage>3.64)
            return 45;
        else if(voltage>3.63)
            return 40;
        else if(voltage>3.62)
            return 35;
        else if(voltage>3.59)
            return 30;
        else if(voltage>3.58)
            return 25;
        else if(voltage>3.57)
            return 20;
        else if(voltage>3.55)
            return 15;
        else if(voltage>3.52)
            return 10;
        else if(voltage>3.5)
            return 5;
        else
            return 0;
        
    }

    void updateHour()
    {
        std::string data = send("AT+CCLK?", "+CCLK:");

        std::cout << data << std::endl;

        // Find the start and end positions of the date and time string
        size_t start = data.find("\"");
        if (start == std::string::npos) {
            return;
        }
        start++;

        size_t end = data.find("+");
        if (end == std::string::npos) {
            return;
        }

        // Extract the date and time string
        std::string dateTime = data.substr(start, end - start);

        // Extract the year, month, and day
        try {
            years = std::atoi(dateTime.substr(0, 2).c_str());
            months = std::atoi(dateTime.substr(3, 2).c_str());
            days = std::atoi(dateTime.substr(6, 2).c_str());
        } catch (const std::invalid_argument&) {
            return;
        }

        // Extract the hour, minute, and second
        try {
            hours = std::atoi(dateTime.substr(9, 2).c_str());
            minutes = std::atoi(dateTime.substr(12, 2).c_str());
            seconds = std::atoi(dateTime.substr(15, 2).c_str());
        } catch (const std::invalid_argument&) {
            return;
        }

        std::cout << years << "-" << months << "-" << days << " " << hours << ":" << minutes << ":" << seconds << std::endl;
    }

    void getHour()
    {
        requests.push_back({std::bind(&GSM::updateHour), priority::high});
    }

    void run()
    {
        init();

        requests.push_back({[](){ send("AT+CNTP=\"time.google.com\",8", "AT+CNTP"); send("AT+CNTP","AT+CNTP", 1000); }, priority::high});

        updateHour();

        eventHandlerBack.setInterval(new Callback<>(&GSM::getHour), 5000);
        eventHandlerBack.setInterval(new Callback<>([](){ requests.push_back({&GSM::getVoltage, GSM::priority::normal}); }), 5000);

        //eventHandlerBack.setInterval(new Callback<>([](){if(send("AT", "AT").find("OK") == std::string::npos) init(); }), 15000);

        keys.push_back({"RING", &GSM::onRinging});
        keys.push_back({"+CMTI:", &GSM::onMessage});
        keys.push_back({"VOICE CALL: END", &GSM::onHangOff});
        keys.push_back({"VOICE CALL: BEGIN", [](){ state.callState = CallState::CALLING; }});


        while (true)
        {
            PaxOS_Delay(50);

            download();

            process();
            data="";

            checkRequest();
        }
    }
};
