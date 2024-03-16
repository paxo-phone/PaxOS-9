#include "gsm.hpp"

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
            gsm.println("AT+IPR=9600\r");   // configure baud rate to 9600
            gsm.begin(9600, SERIAL_8N1, RX, TX);

            while(gsm.available())
                gsm.read();
            
            gsm.println("AT\r");
            delay(1000);

            if (gsm.available())
            {
                std::cout << "Connected" << std::endl;
                return;
            }
        }
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

                if(answer.find(answerKey) == std::string::npos)
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
            auto it = requests.begin();
            while (it != requests.end())
            {
                if (it->priority == pr)
                {
                    it->function();
                    it = requests.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
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
        requests.push_back({std::bind(&GSM::sendMessage, number, message), priority::normal});
    }

    void sendCall(const std::string &number)
    {
        if(send("ATD" + number + ";", "ATD").find("OK") != std::string::npos)
        {
            state.callState = CallState::CALLING;
            state.callingNumber = number;
        }
    }

    void newCall(std::string number)
    {
        requests.push_back({std::bind(&GSM::sendCall, number), priority::high});
    }

    void run()
    {
        init();

        keys.push_back({"RING", &GSM::onRinging});
        keys.push_back({"+CMTI:", &GSM::onMessage});
        keys.push_back({"VOICE CALL: END", &GSM::onHangOff});
        keys.push_back({"VOICE CALL: BEGIN", [](){ state.callState = CallState::NOT_CALLING; }});

        while (true)
        {
            delay(100);
            download();

            process();
            data="";

            checkRequest();
        }
    }
};
