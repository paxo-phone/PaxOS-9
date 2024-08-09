#include "gsm.hpp"
#include "contacts.hpp"
#include "conversation.hpp"
#include <path.hpp>
#include <filestream.hpp>
#include <iostream>
#include <ctime>
#include <codecvt>
#include <delay.hpp>
#include "../../tasks/src/threads.hpp"

const char *daysOfWeek[7] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
const char *daysOfMonth[12] = {"Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"};

#ifdef ESP_PLATFORM
#include <Arduino.h>
#include <HardwareSerial.h>
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "esp_system.h"

#define RX 26
#define TX 27

#define GSM_UART UART_NUM_2
#define gsm Serial2
#endif

#include <delay.hpp>


namespace GSM
{
    std::string data;
    std::vector<Request> requests;
    std::vector<Key> keys;
    std::vector<Message> messages;
    State state;
    uint16_t seconds, minutes, hours, days, months, years = 0;
    float voltage = -1;
    int networkQuality = 0;

    namespace ExternalEvents
    {
        std::function<void(void)> onIncommingCall;
        std::function<void(void)> onNewMessage;
        std::function<void(void)> onNewMessageError;
    }

    void init()
    {
#ifdef ESP_PLATFORM
        pinMode(32, OUTPUT); // define pin mode

        digitalWrite(32, 1); // power on the module
        delay(1000);
        digitalWrite(32, 0);

        gsm.begin(115200, SERIAL_8N1, RX, TX);

        // Get the UART number (in this case, UART2 for Serial2)
        uart_port_t uart_num = UART_NUM_2;

        // Configure UART to use REF_TICK as clock source
        UART2.conf0.tick_ref_always_on = 1;

        // Set the UART clock divider
        uint32_t sclk_freq = 1000000; // REF_TICK frequency is 1MHz
        uint32_t baud_rate = 115200;  // Or whatever baud rate you're using
        uint32_t clk_div = ((sclk_freq + baud_rate / 2) / baud_rate);

        UART2.clk_div.div_int = clk_div;
        UART2.clk_div.div_frag = 0;

        // Reconfigure the baud rate
        Serial2.updateBaudRate(baud_rate);

        while (true)
        {

            while (gsm.available())
                gsm.read();

            gsm.println("AT\r");
            delay(1000);

            if (gsm.available())
            {
                std::cout << "[GSM] Connected!" << std::endl;
                return;
            }
            else
                std::cout << "[GSM] [Error] Disconnected" << std::endl;
        }
#endif
    }

    void reInit()
    {
#ifdef ESP_PLATFORM
        download();

        // Get the UART number (in this case, UART2 for Serial2)
        uart_port_t uart_num = UART_NUM_2;

        // Configure UART to use REF_TICK as clock source
        UART2.conf0.tick_ref_always_on = 1;

        // Set the UART clock divider
        uint32_t sclk_freq = 1000000; // REF_TICK frequency is 1MHz
        uint32_t baud_rate = 115200;  // Or whatever baud rate you're using
        uint32_t clk_div = ((sclk_freq + baud_rate / 2) / baud_rate);

        UART2.clk_div.div_int = clk_div;
        UART2.clk_div.div_frag = 0;

        // Reconfigure the baud rate
        Serial2.updateBaudRate(baud_rate);
#endif
    }

    void download(uint64_t timeout)
    {
#ifdef ESP_PLATFORM
        uint64_t timer = 0;
        while (gsm.available() || timer + timeout > millis())
        {
            if (gsm.available())
            {
                timer = millis();
                data += gsm.read();
            }
        }
#endif
    }

    std::string send(const std::string &message, const std::string &answerKey, uint64_t timeout)
    {
#ifdef ESP_PLATFORM
        gsm.println((message + "\r").c_str());

        std::cout << "[GSM] Sending request" << std::endl;

        uint64_t lastChar = millis();
        std::string answer = "";

        while (lastChar + timeout > millis()) // save none related messages to data.
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

        while (lastChar + timeout > millis() && (answer.find("OK\r\n") == std::string::npos && answer.find("ERROR\r\n") == std::string::npos))
        {
            if (gsm.available())
            {
                answer += gsm.read();
                lastChar = millis();
            }
        }

        /*if(lastChar + timeout < millis())
        {
            std::cerr << "[GSM] Response timeout: " << answer  << std::endl;
        }
        else
        {
            std::cout << "[GSM] Response: " << answer << std::endl;
        }*/

        return answer;
#endif

        return "";
    }

    void appendRequest(Request request)
    {
        // ask the other core to add a request
        if (!request.function)
            std::cout << "request.function is invalid -> can't run the new request" << std::endl;
        else
            eventHandlerBack.setTimeout(new Callback<>(std::bind([](Request r){ GSM::requests.push_back(r); }, request)), 0);
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
        for (uint8_t pr = priority::high; pr <= priority::low; pr++)
        {
            auto it = 0;
            while (it != requests.size())
            {
                if (requests[it].priority == pr)
                {
                    if (requests[it].function != nullptr)
                        requests[it].function();
                    requests[it].function = nullptr;
                }
                it++;
            }
        }

        requests.clear();
    }

    void clearFrom(const std::string &from, const std::string &to)
    {
        int first = data.find(from);
        if (first == std::string::npos)
        {
            return;
        }

        int end = data.find(to, first);
        if (end == std::string::npos)
        {
            return;
        }

        std::string before = data.substr(0, first);
        std::string after = "";
        if (end < data.length() - 1)
        {
            after = data.substr(end + to.length());
        }

        data = before + after;
    }

    void onRinging()
    {
        if (state.callState == CallState::RINGING)
            return;

        std::string defaultNumber = "unknown";

        int clccIndex = data.find("+CLCC:");
        if (clccIndex == std::string::npos)
        {
            state.callingNumber = defaultNumber;
        }

        int quoteIndex = data.find("\"", clccIndex);
        if (quoteIndex == std::string::npos)
        {
            state.callingNumber = defaultNumber;
        }

        int endQuoteIndex = data.find("\"", quoteIndex + 1);
        if (endQuoteIndex == std::string::npos)
        {
            state.callingNumber = defaultNumber;
        }

        state.callingNumber = data.substr(quoteIndex + 1, endQuoteIndex - quoteIndex - 1);
        state.callState = RINGING;

        clearFrom("RING", "\"\"");
        std::cout << "Number is calling: \"" << state.callingNumber << "\"" << std::endl;

        if (ExternalEvents::onIncommingCall)
            ExternalEvents::onIncommingCall();
    }

    void onHangOff()
    {
        state.callState = NOT_CALLING;

        clearFrom("VOICE CALL: END", "NO CARRIER");

        std::cout << "hanging off" << std::endl;
    }

    std::string getCurrentTimestamp()
    {
        char buf[20];
        std::sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", GSM::years, GSM::months, GSM::days, GSM::hours, GSM::minutes, GSM::seconds);
        return std::string(buf);
    }

    std::string getCurrentTimestampNoSpaces()
    {
        char buf[20];
        std::sprintf(buf, "%04d-%02d-%02d_%02d:%02d:%02d", GSM::years, GSM::months, GSM::days, GSM::hours, GSM::minutes, GSM::seconds);
        return std::string(buf);
    }

    bool is_hex_string(const std::string& str) {
        return str.length() % 4 == 0 && str.find_first_not_of("0123456789ABCDEFabcdef") == std::string::npos;
    }

    std::string convert_hex_to_utf8(const std::string& hex_str) {
        std::u16string utf16_str;
        for (size_t i = 0; i < hex_str.length(); i += 4) {
            std::string hex_char = hex_str.substr(i, 4);
            char16_t utf16_char = static_cast<char16_t>(std::stoul(hex_char, nullptr, 16));
            utf16_str.push_back(utf16_char);
        }
        
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
        return converter.to_bytes(utf16_str);
    }

    std::string process_string(const std::string& input) {
        if (is_hex_string(input)) {
            return convert_hex_to_utf8(input);
        } else {
            return input; // Already in UTF-8
        }
    }

    void onMessage()
    {
        std::cout << "onMessage data: " << data << std::endl;
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
            k = str.find("\"", k + 1);
            k = str.find("\"", k + 1);

            number = str.substr(k + 1, str.find("\"", k + 1) - k - 1);

            k = str.find("\"", k + 1);
            k = str.find("\"", k + 1);
            k = str.find("\"", k + 1);
            k = str.find("\"", k + 1);

            date = str.substr(k + 1, str.find("\"", k + 1) - k - 4);

            k = str.find("\"", k + 1);

            std::string message2 = str.substr(k + 2 + 1, -1);
            std::replace(message2.begin(), message2.end(), '\n', 'n');
            std::replace(message2.begin(), message2.end(), '\r', 'r');

            message = str.substr(k + 3, str.find("\r\n", k + 3) - k - 3);

            // Vérifier si le numéro existe dans les contacts
            if(number.size() == 10) number = "+33" + number.substr(1);
            message = process_string(message);

            auto contact = Contacts::getByNumber(number);

            if (contact.name.empty())
            {
                std::cout << "Message from unknown number: " << number << std::endl;
            }
            else
            {
                std::cout << "Message from contact: " << contact.name << std::endl;
            }

            // Ajout du message à la conversation
            Conversations::Conversation conv;
            storage::Path convPath(std::string(MESSAGES_LOCATION) + "/" + number + ".json");
            if (convPath.exists())
            {
                Conversations::loadConversation(convPath, conv);
            }
            else
            {
                conv.number = number;
            }
            conv.messages.push_back({message, true, getCurrentTimestamp()}); // true = message de l'autre
            Conversations::saveConversation(convPath, conv);

            std::cout << "New message: " << number << " - " << message << std::endl;

            storage::FileStream file(std::string(MESSAGES_NOTIF_LOCATION), storage::Mode::READ);
            std::string content = file.read();
            file.close();

            std::cerr << content << std::endl;

            if(content.find(number) == std::string::npos)
            {
                storage::FileStream file2(storage::Path(std::string(MESSAGES_NOTIF_LOCATION)).str(), storage::Mode::APPEND);
                file2.write(number + "\n");
                file2.close();
            }

            i = j + 1;
        }

        send("AT+CMGD=1,3", "AT+CMGD", 1000);

        if (ExternalEvents::onNewMessage)
            ExternalEvents::onNewMessage();
    }

    void sendMessage(const std::string &number, const std::string &message)
    {
        bool sent = false;

        for (int i = 0; i<3 && !sent; i++)
        {
            int step = 0;
            std::string result = "";

            std::cout << "Sending message to: " << number << " with content: " << message << std::endl;

            for (int i = 0; i<2; i++)
            {
                if((result = send("AT+CMGF=1", "OK", 9000)).find("OK") == std::string::npos)
                {
                    std::cout << "Sending message failed (step 1), logs: " << result << std::endl;
                }
                else
                {
                    step++;
                    break;
                }
            }

            if(step == 1)
            {
                for (int i = 0; i<1; i++)
                {
                    if((result = send("AT+CMGS=\"" + number + "\"\r", ">", 1000)).find(">") == std::string::npos)
                    {
                        std::cout << "Sending message failed (step 2), logs: " << result << std::endl;
                    }
                    else
                    {
                        step++;
                        break;
                    }
                }
            }

            if(step == 2)
            {
                for (int i = 0; i<3; i++)
                {
                    if((result = send(message + char(26), "OK", 5000)).find("OK") == std::string::npos)
                    {
                        std::cout << "Sending message failed (step 3), logs: " << result << std::endl;
                    }
                    else
                    {
                        sent = true;
                        break;
                    }
                }
            }

            std::cout << "did " << step << " steps" << std::endl;
        }
        
        if(sent)
        {
            Conversations::Conversation conv;
            storage::Path convPath(std::string(MESSAGES_LOCATION) + "/" + number + ".json");
            if (convPath.exists())
            {
                Conversations::loadConversation(convPath, conv);
            }
            else
            {
                conv.number = number;
            }
            conv.messages.push_back({message, false, getCurrentTimestamp()}); // false = message de l'user
            Conversations::saveConversation(convPath, conv);
        }
        else
        {
            if(ExternalEvents::onNewMessageError)
                ExternalEvents::onNewMessageError();

            // todo: timeout to retry later
        }
    }

    void newMessage(std::string number, std::string message)
    {
        GSM::messages.push_back({number, message, ""});
        appendRequest({std::bind(&GSM::sendMessage, number, message), priority::normal});
    }

    void sendCall(const std::string &number)
    {
        std::cout << "Calling " << number << std::endl;
        if (send("ATD" + number + ";", "OK", 2000).find("OK") != std::string::npos)
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

    float getVoltage()
    {
        std::string answer = send("AT+CBC", "OK");

        int start = answer.find("+CBC: ") + 6;
        int end = answer.find("V", start);

        if (start == std::string::npos || end == std::string::npos) // maybe wrong
            return 0;

        std::string voltage_str = answer.substr(start, end - start);

        try
        {
            return std::stof(voltage_str);
        }
        catch (std::exception)
        {
            return 0;
        }
    }

    int getBatteryLevel()
    {
        float voltage = getVoltage();

        if(voltage == -1)
            return 100;
        if (voltage > 4.12)
          return 100;
        else if (voltage > 4.03)
            return 95;
        else if (voltage > 3.99)
            return 90;
        else if (voltage > 3.94)
            return 85;
        else if (voltage > 3.90)
            return 80;
        else if (voltage > 3.86)
            return 75;
        else if (voltage > 3.82)
            return 70;
        else if (voltage > 3.77)
            return 65;
        else if (voltage > 3.74)
            return 60;
        else if (voltage > 3.70)
            return 55;
        else if (voltage > 3.66)
            return 50;
        else if (voltage > 3.64)
            return 45;
        else if (voltage > 3.63)
            return 40;
        else if (voltage > 3.62)
            return 35;
        else if (voltage > 3.59)
            return 30;
        else if (voltage > 3.58)
            return 25;
        else if (voltage > 3.57)
            return 20;
        else if (voltage > 3.55)
            return 15;
        else if (voltage > 3.52)
            return 10;
        else if (voltage > 3.5)
            return 5;
        else
            return 0;
    }

    void updateHour()
    {
        std::string data = send("AT+CCLK?", "+CCLK:");

        size_t start = data.find("\"");
        if (start == std::string::npos)
        {
            return;
        }
        start++;

        size_t end = data.find("+");
        if (end == std::string::npos)
        {
            return;
        }

        std::string dateTime = data.substr(start, end - start);

        // Extract the year, month, and day
        try
        {
            years = std::atoi(dateTime.substr(0, 2).c_str());
            months = std::atoi(dateTime.substr(3, 2).c_str());
            days = std::atoi(dateTime.substr(6, 2).c_str());
        }
        catch (const std::invalid_argument &)
        {
            return;
        }

        // Extract the hour, minute, and second
        try
        {
            hours = std::atoi(dateTime.substr(9, 2).c_str());
            minutes = std::atoi(dateTime.substr(12, 2).c_str());
            seconds = std::atoi(dateTime.substr(15, 2).c_str());
        }
        catch (...)
        {
            return;
        }

        //std::cout << years << "-" << months << "-" << days << " " << hours << ":" << minutes << ":" << seconds << std::endl;
    }

    void getHour()
    {
        requests.push_back({std::bind(&GSM::updateHour), priority::high});
    }

    int getNetworkStatus()
    {
        return networkQuality;
    }

    void updateNetworkQuality()
    {
        std::string o = send("AT+CSQ", "OK");
        if(o.find("+CSQ:") != std::string::npos)
        {
            networkQuality = atoi(o.substr(o.find("+CSQ: ") + 5, o.find(",") - o.find("+CSQ: ") - 5).c_str());
        }
        std::cout << "networkQuality: " << networkQuality << std::endl;
    }

    void getNetworkQuality()
    {
        requests.push_back({&GSM::updateNetworkQuality, priority::normal});
    }

    void run()
    {
        init();

        //PaxOS_Delay(50000);

        requests.push_back({[](){ send("AT+CNTP=\"time.google.com\",8", "AT+CNTP"); send("AT+CNTP","AT+CNTP", 1000); }, priority::high});

        updateHour();
        getNetworkQuality();
        onMessage();

        eventHandlerBack.setInterval(&GSM::getHour, 5000);
        eventHandlerBack.setInterval(&GSM::getNetworkQuality, 10000);
        eventHandlerBack.setInterval([](){ requests.push_back({&GSM::getVoltage, GSM::priority::normal}); }, 5000);
        // eventHandlerBack.setInterval(new Callback<>([](){if(send("AT", "AT").find("OK") == std::string::npos) init(); }), 15000);

        keys.push_back({"RING", &GSM::onRinging});
        keys.push_back({"+CMTI:", &GSM::onMessage});
        keys.push_back({"VOICE CALL: END", &GSM::onHangOff});
        keys.push_back({"VOICE CALL: BEGIN", [](){ state.callState = CallState::CALLING; }});

        while (true)
        {
            PaxOS_Delay(5);

            download();

            process();
            data = "";

            checkRequest();
        }
    }
};
