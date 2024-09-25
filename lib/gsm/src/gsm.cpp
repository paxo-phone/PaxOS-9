#include "gsm.hpp"

#include <sstream>
#include <iostream>
#include <ctime>
#include <codecvt>

#include <path.hpp>
#include <filestream.hpp>
#include <imgdec.hpp>
#include <delay.hpp>
#include <threads.hpp>
#include <graphics.hpp>
#include <standby.hpp>

#include "Image.hpp"
#include "Surface.hpp"

#include "contacts.hpp"
#include "conversation.hpp"
#include "pdu.hpp"

const char *daysOfWeek[7] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
const char *daysOfMonth[12] = {"Janvier", "Fevrier", "Mars", "Avril", "Mai", "Juin", "Juillet", "Aout", "Septembre", "Octobre", "Novembre", "Decembre"};

EventHandler eventHandlerGsm;

#ifdef ESP_PLATFORM
#include <Arduino.h>

#define RX 26
#define TX 27

#define gsm Serial2
#endif

namespace GSM
{
    std::string data;
    std::vector<Request> requests;
    std::vector<Key> keys;
    std::vector<Message> messages;
    State state;
    uint16_t seconds, minutes, hours, days, months, years = 0;
    std::vector<float> battery_voltage_history;
    float voltage = -1;
    int networkQuality = 0;
    bool flightMode = false;

    namespace ExternalEvents
    {
        std::function<void(void)> onIncommingCall;
        std::function<void(void)> onNewMessage;
        std::function<void(void)> onNewMessageError;
    }

    void init()
    {
#ifdef ESP_PLATFORM

        Serial.println("GSM initializing");
        pinMode(32, OUTPUT); // define pin mode

        /*digitalWrite(32, 1); // power on the module
        delay(60);  // according to the datasheet t > 50ms
        digitalWrite(32, 0);*/

        gsm.setRxBufferSize(1024*30);

        bool rebooted = false;
        
        while(true)
        {
            gsm.begin(115200, SERIAL_8N1, RX, TX);
            gsm.println("AT\r");
            delay(100);
            String data = gsm.readString();
            if (data.indexOf("OK") != -1)
            {
                Serial.println("GSM responding at 115200");

                gsm.println("AT+IPR=921600");
                gsm.flush();
                gsm.updateBaudRate(921600);

                return;
            }
            else
            {
                Serial.println("GSM not responding at 115200, trying 921600");
                Serial.println(data);

                gsm.updateBaudRate(921600);
            
                gsm.println("AT\r");
                delay(100);
                String data = gsm.readString();
                if (data.indexOf("OK") != -1)
                {
                    Serial.println("GSM responding at 921600");
                    return;
                }
                else
                {
                    Serial.println("GSM not responding at 115200 and 921600");
                    Serial.println(data);


                    if(!rebooted)
                    {
                        Serial.println("Powering on the module");
                        digitalWrite(32, 1); // power on the module
                        delay(60);  // according to the datasheet t > 50ms
                        digitalWrite(32, 0);
                        rebooted = true;
                    }
                }
            }
            gsm.end();
        }
#endif
    }

    void reInit()
    {
#ifdef ESP_PLATFORM
        download();

        gsm.updateBaudRate(BAUDRATE);
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

        std::cout << "[GSM] Sending request: " << message << ", " << answerKey << std::endl;

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
            eventHandlerGsm.setTimeout(new Callback<>(std::bind([](Request r){ GSM::requests.push_back(r); }, request)), 0);
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
        std::sprintf(buf, "%04d-%02d-%02d_%02d-%02d-%02d", GSM::years, GSM::months, GSM::days, GSM::hours, GSM::minutes, GSM::seconds);
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

    /*void onMessage()
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
    }*/

    #ifdef ESP_PLATFORM

    namespace Stream {
        struct Chunk
        {
            char* data;
            int size;
        };

        std::vector<Chunk> chunks;
        bool running = false;

        void stream(storage::FileStream& stream, int size) {
            running = true;

            std::cout << "Streaming chunks: " << size << std::endl;

            while (running)
            {
                while ((chunks.empty() && running)) {
                    delay(5);
                }

                if(!chunks.empty())
                {
                    std::cout << "Writing chunk of size: " << int(chunks[0].size/1000) << "Ko" << std::endl;
                    stream.write(chunks[0].data, chunks[0].size);
                    delete[] chunks[0].data;
                    chunks.erase(chunks.begin());
                    std::cout << "Writing chunk ended: " << int(chunks[0].size/1000) << "Ko" << std::endl;
                }
            }
        }

        void end()
        {
            running = false;
            while(!chunks.empty()) delay(1);
        }

        void addChunk(char* chunk, int size) {
            chunks.push_back({chunk, size});
        }
    }

    #endif

    // MMS: download a MMS from a URL, then decode the jpeg in it, and add it to the related conversation
    std::string getHttpMMS(std::string number, std::string url) {
        #ifdef ESP_PLATFORM
        StandbyMode::triggerPower();

        // sending the AT command to initialize the HTTP session
        send("AT+HTTPINIT", "AT+HTTPINIT", 500);
        send("AT+HTTPPARA=\"URL\",\"" + url + "\"\r", "AT+HTTPPARA", 500);
        send("AT+HTTPACTION=0", "AT+HTTPACTION", 500);

        // recovering the size of the data
        bool result = false;
        uint64_t timeout = millis() + 10000;
        while (millis() < timeout) {
            StandbyMode::triggerPower();
            download();

            uint32_t i = data.find("+HTTPACTION: ");
            if(i != std::string::npos)
            {
                if(data.find("\r\n", i) != std::string::npos)
                {
                    result = true;
                    std::cout << "Request to mms succeeded" << std::endl;
                    break;
                }
            }
        }

        if (!result || data.find("+HTTPACTION: 0,200,") == std::string::npos) {
            std::cerr << "Request to mms failed" << std::endl;
            std::cout << data << std::endl;
            return "";
        }

        std::string sizedata = "";

        sizedata = data.substr(data.find("+HTTPACTION: 0,200,") + sizeof("+HTTPACTION: 0,200,") - 1);
        sizedata = sizedata.substr(0, sizedata.find("\r\n"));

        std::cout << "Size of the Data 0: " << data << std::endl;

        std::cout << "Size of the Data: " << sizedata << std::endl;
        int size = atoi(sizedata.c_str());


        // initialisation des fichiers
        auto list = storage::Path(MESSAGES_IMAGES_LOCATION).listdir();  // liste les images déjà présentes
        std::sort(list.begin(), list.end());    // trier par ordre croissant
        int lastIndex = (list.size()==0) ? 0 : atoi(list[list.size() - 1].substr(0, list[list.size() - 2].find("p.jpg")).c_str());   // récupere l'index de l'image la plus recente
        std::string filename = std::to_string(++lastIndex) + ".jpg";    // ajoute a l'index 1, et voila le nom de l'image
        std::string filename_preview = std::to_string(lastIndex) + "p.jpg";    // ajoute a l'index 1, et voila le nom de l'image

        while (filename.size() < 8) filename = "0" + filename;
        while (filename_preview.size() < 9) filename_preview = "0" + filename_preview;

        std::cout << "List of images: ";
        for (auto& file : list) {
            std::cout << file << " ";
        }
        std::cout << std::endl;

        storage::FileStream filestream((storage::Path(MESSAGES_IMAGES_LOCATION) / filename).str(), storage::Mode::WRITE); // ouvre le fichier de l'image en écriture
        eventHandlerApp.setTimeout(new Callback<>([&](){ Stream::stream(filestream, size / 1024 + 1); }), 0);   // ouvre un autre thread pour le stream
        StandbyMode::triggerPower();

        gsm.readString();   // vide le buffer
        
        uint64_t timer = millis();  // pour le timeout
        uint64_t timeout_block = 10000;    // timeout de 1 secondes

        uint8_t jpg = 0;    // 0 = no jpeg, 1 = jpeg found, 2 = jpeg done
        char lastChar = 0;  // dernier caractère lu
        int bufferIndex = 0;    // index d'écriture dans le buffer
        int bufferSize = 1024 * 100;    // taille du buffer
        int blockIndex = 0;    // index de lecture du bloc
        int blockSize = 1024;    // taille d'un bloc
        int numberOfBlocks = 10;    // nombre de blocs à lire en une fois (dépend de la taille du buffer serie pour éviter les overflows)
        char* buffer = new char[bufferSize];    // allocation du buffer de lecture // ne contient que du jpeg

        gsm.println("AT+HTTPREAD=0," + String(1024*numberOfBlocks) + "\r");    // Requette au module pour envoyer les premiers blocs de données

        graphics::Surface loading(320, 5);    // affiche le chargement parce que l'app en cours est figée
            loading.fillRect(0, 0, loading.getWidth(), loading.getHeight(), 0xFFFF);
        
        for(uint32_t i = 0; i < size;) {    // pour tous les caractères de données brutes annoncés
            for (uint32_t r = 0; r < numberOfBlocks +1 && i < size; r++)    // pour tous les blocs envoyés en une fois
            {
                while ((gsm.available() && timer + timeout_block > millis())?(gsm.read() != ':'):(true));   // wait for the garbage data to be ignored
                while ((gsm.available() && timer + timeout_block > millis())?(gsm.read() != '\n'):(true));

                if(r == numberOfBlocks)
                    break;

                int nextBlockSize = (size - i >= blockSize) ? 1024 : (size - i);    // size of the current block, that is equal or less than 1024
                while (gsm.available() < nextBlockSize && timer + timeout_block > millis());    // wait for the next block to be downloaded

                timer = millis();

                if(jpg == 0)    // no jpeg for the moment
                {
                    for (uint32_t j = 0; j < nextBlockSize; j++)    // parse the block
                    {
                        char c = gsm.read();    // read the next char
                        
                        if(lastChar == '\xFF' && c == '\xD8')   // if a jpg header is found
                        {
                            std::cout << "Found JPEG" << std::endl;

                            jpg = 1;                // set the state to reading jpg
                            buffer[0] = lastChar;   // 0xFF
                            buffer[1] = c;          // 0xD8

                            gsm.readBytes(buffer+2, nextBlockSize - j - 1); // read the rest of the block into the buffer -> buffer = 0xFF 0xD8 [rest of the data...]
                            bufferIndex = 2 + nextBlockSize - j - 1;        // set the writing index to the size that has been written
                            break;                // no need to search for jpg in the rest of the block
                        }

                        lastChar = c;    // save the last char if the jpg header is split in two blocks
                    }
                }
                else if(jpg == 1)   // if is reading the jpg
                {
                    if(bufferIndex + blockSize*2 >= bufferSize)   // if the buffer is full, create a new one, and send the other to the stream
                    {
                        Stream::addChunk(buffer, bufferIndex);  // add the buffer to the stream
                        bufferIndex = 0;                        // reset the index
                        buffer = new char[bufferSize];    // create a new buffer
                    }

                    gsm.readBytes(buffer + bufferIndex, nextBlockSize); // read the next block, and add it to the buffer (after the last block so ```+ bufferIndex```)

                    for (int j = (bufferIndex==0)?(0):(-1); j < nextBlockSize - 1 && bufferIndex + j + 1 < bufferSize; j++)    // search for the end header; initialise j to -1 if the buffer is not empty, so it can check the header in the last char of the last buffer
                    {
                        if(buffer[bufferIndex + j] == 0xFF && buffer[bufferIndex + j + 1] == 0xD9)   // if the end header is found
                        {
                            std::cout << "End of JPEG" << std::endl;

                            jpg = 2;            // set the state to ignore the rest of the data
                            bufferIndex += j + 2;    // set the writing index to the end of the end header, so ignore the rest of the data

                            Stream::addChunk(buffer, bufferIndex);  // add the buffer to the stream
                            bufferIndex = 0;
                            
                            break;
                        }
                    }

                    if(jpg == 1)    // if no end header is found, adding the size of the block to the writing index
                        bufferIndex += nextBlockSize;   
                } else  // jpg == 2
                {
                    for (uint32_t j = 0; j < nextBlockSize; j++)    // ignore the rest of the data
                        gsm.read();
                }

                i += nextBlockSize;
            }

            gsm.println("AT+HTTPREAD=0," + String(1024*numberOfBlocks) + "\r");    // Read the data

            // loading bar in the terminal
            std::cout << "[";
            for (uint32_t j = 0; j < 20; j++)
            {
                if(i < j*size/20)
                    std::cout << " ";
                else
                    std::cout << "=";
            }
            std::cout << "] " << int(i/1024) << "Ko" << std::endl;

            // update the graphical update bar
            loading.fillRect(0, 0, 320 * i / size, loading.getHeight(), 0);
            graphics::showSurface(&loading, 0, 480-5);

            StandbyMode::triggerPower();
        }

        Stream::end();
        filestream.close();
        send("AT+HTTPTERM", "AT+HTTPTERM", 100);    // close http connection

        StandbyMode::triggerPower();

        ///////////////////////////////////////// FULLSCREEN IMAGE PROCESSING /////////////////////////////////////////

        storage::Path path = (storage::Path(MESSAGES_IMAGES_LOCATION) / filename);  // path to the new image

        graphics::SImage i = graphics::SImage(path);    // get the size of the image
        uint16_t m_width = i.getWidth();
        uint16_t m_height = i.getHeight();

        float scale_width = 320.0 / m_width;    // calculate the scaling factor of the image to fit in the screen
        float scale_height = 480.0 / m_height;
        float scale = std::min(scale_width, scale_height);

        graphics::Surface sprite(m_width * scale, m_height * scale, 24);    // create a surface to draw the image on, to compress it then
        sprite.m_sprite.drawJpgFile(path.str().c_str(), 0, 0, 0, 0, 0, 0, scale, scale);    // draw the image (can take several seconds)

        StandbyMode::triggerPower();

        imgdec::encodeJpg(reinterpret_cast<uint8_t*>(sprite.m_sprite.getBuffer()), sprite.getWidth(), sprite.getHeight(), (storage::Path(MESSAGES_IMAGES_LOCATION) / filename));    // compress the image and save it (take less than 2s)

        sprite.m_sprite.deleteSprite();
        
        ///////////////////////////////////////// PREVIEW IMAGE PROCESSING /////////////////////////////////////////

        scale_width = 60.0 / 320;    // calculate the scaling factor of the image to fit in the preview
        scale_height = 60.0 / 480;
        scale = std::min(scale_width, scale_height);

        StandbyMode::triggerPower();

        graphics::Surface sprite_preview(60, 60, 24);    // create a surface to draw the image on, to compress it then
        sprite_preview.fillRect(0, 0, 60, 60, 0xFFFF);
        sprite_preview.m_sprite.drawJpgFile(path.str().c_str(), 0, 0, 0, 0, 0, 0, scale, scale);    // draw the image (can take several seconds)
        //graphics::showSurface(&sprite_preview, 0, 0);

        StandbyMode::triggerPower();

        path = (storage::Path(MESSAGES_IMAGES_LOCATION) / filename_preview);  // path to the new image preview
        imgdec::encodeJpg(reinterpret_cast<uint8_t*>(sprite_preview.m_sprite.getBuffer()), sprite_preview.getWidth(), sprite_preview.getHeight(), path);    // compress the image and save it (take less than 2s)
        
        sprite_preview.m_sprite.deleteSprite();

        ////////////////////////////////////////// END /////////////////////////////////////////

        // same procedure as saving message
        auto contact = Contacts::getByNumber(number);

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

        conv.messages.push_back({"/" + filename, true, getCurrentTimestamp()}); // true = message de l'autre
        Conversations::saveConversation(convPath, conv);

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

        #endif

        return "";
    }

    void onMessage()
    {
        send("AT+CMGF=0", "AT+CMGF=0", 100);

        std::string input = send("AT+CMGL=0", "AT+CMGL", 500);

        std::vector<std::string> pdus;

        std::istringstream iss(input);
        std::string line;

        while (std::getline(iss, line)) {
            // Remove \r if present at the end of the line
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            // Check if the line starts with "+CMGL:"
            if (line.find("+CMGL:") == 0) {
                // The next line should contain the PDU
                if (std::getline(iss, line)) {
                    // Remove \r if present at the end of the line
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }
                    // Remove any whitespace
                    line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
                    // Add the PDU to the vector
                    pdus.push_back(line);
                }
            }
        }

        // Print the extracted PDUs
        for (const auto& pdu : pdus) {
            std::cout << pdu << std::endl;
        }

        for (auto pdu : pdus)
        {
            std::cout << "PDU: " << pdu << std::endl;
            try
            {
                PDU decoder = decodePDU(pdu);

                if(decoder.type != PDU_type::UNKNOWN)
                {
                    std::cout << "Phone number: " << decoder.sender << std::endl;

                    if(decoder.type == PDU_type::MMS)
                    {
                        std::cout << "MMS: " << decoder.url << std::endl;
                        std::string number = decoder.sender;
                        std::string link = decoder.url;
                        getHttpMMS(number, link);
                    }
                    else // PDU_type::SMS
                    {
                        std::cout << "Message: " << decoder.message << std::endl;

                        auto contact = Contacts::getByNumber(decoder.sender);

                        // Ajout du message à la conversation
                        Conversations::Conversation conv;
                        storage::Path convPath(std::string(MESSAGES_LOCATION) + "/" + decoder.sender + ".json");
                        if (convPath.exists())
                        {
                            Conversations::loadConversation(convPath, conv);
                        }
                        else
                        {
                            conv.number = decoder.sender;
                        }

                        conv.messages.push_back({decoder.message, true, getCurrentTimestamp()}); // true = message de l'autre
                        Conversations::saveConversation(convPath, conv);

                        storage::FileStream file(std::string(MESSAGES_NOTIF_LOCATION), storage::Mode::READ);
                        std::string content = file.read();
                        file.close();

                        std::cerr << content << std::endl;

                        if(content.find(decoder.sender) == std::string::npos)
                        {
                            storage::FileStream file2(storage::Path(std::string(MESSAGES_NOTIF_LOCATION)).str(), storage::Mode::APPEND);
                            file2.write(decoder.sender + "\n");
                            file2.close();
                        }
                    }
                }
            }
            catch (const std::out_of_range& e) {
                std::cerr << "Erreur : " << e.what() << std::endl;
            }
        }

        if(pdus.size())
        {
            if (ExternalEvents::onNewMessage)
                ExternalEvents::onNewMessage();
        }

        send("AT+CMGD=1,1", "AT+CMGD", 1000);
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

    void getVoltage()       // THIS IS A TASK, DO NOT CALL IT!
    {
        std::string answer = send("AT+CBC", "OK");

        int start = answer.find("+CBC: ") + 6;
        int end = answer.find("V", start);

        if (start == std::string::npos || end == std::string::npos) // maybe wrong
            return;

        std::string voltage_str = answer.substr(start, end - start);

        try
        {
            voltage = std::stof(voltage_str);

            battery_voltage_history.push_back(voltage);
            if (battery_voltage_history.size() > 24)
                battery_voltage_history.erase(battery_voltage_history.begin());

            if (battery_voltage_history.size() > 0) {
                double sum = 0;
                for (auto v : battery_voltage_history)
                    sum += v;
                voltage = sum / battery_voltage_history.size();

                std::cout << "Battery voltage average: " << voltage << std::endl;
            }
        }
        catch (std::exception)
        {
        }
    }

    double getBatteryLevel() {
#ifdef ESP_PLATFORM
        if (voltage == -1) {
            // Probably return something else ?
            return 1;
        }

        // Thanks NumWorks for the regression app
        const double batteryLevel = 3.083368 * std::pow(voltage, 3) - 37.21203 * std::pow(voltage, 2) + 150.5735 * voltage - 203.3347;

        //std::cout << "Battery level: " << batteryLevel << std::endl;

        return std::clamp(batteryLevel, 0.0, 1.0);

        // if (voltage > 4.12)
        //   return 100;
        // else if (voltage > 4.03)
        //     return 95;
        // else if (voltage > 3.99)
        //     return 90;
        // else if (voltage > 3.94)
        //     return 85;
        // else if (voltage > 3.90)
        //     return 80;
        // else if (voltage > 3.86)
        //     return 75;
        // else if (voltage > 3.82)
        //     return 70;
        // else if (voltage > 3.77)
        //     return 65;
        // else if (voltage > 3.74)
        //     return 60;
        // else if (voltage > 3.70)
        //     return 55;
        // else if (voltage > 3.66)
        //     return 50;
        // else if (voltage > 3.64)
        //     return 45;
        // else if (voltage > 3.63)
        //     return 40;
        // else if (voltage > 3.62)
        //     return 35;
        // else if (voltage > 3.59)
        //     return 30;
        // else if (voltage > 3.58)
        //     return 25;
        // else if (voltage > 3.57)
        //     return 20;
        // else if (voltage > 3.55)
        //     return 15;
        // else if (voltage > 3.52)
        //     return 10;
        // else if (voltage > 3.5)
        //     return 5;
        // else
        //     return 0;
#else
        return 1;
#endif
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
        // std::cout << "networkQuality: " << networkQuality << std::endl;
        return networkQuality;
    }

    void updateNetworkQuality()
    {
        std::string o = send("AT+CSQ", "OK");
        if(o.find("+CSQ:") != std::string::npos)
        {
            networkQuality = atoi(o.substr(o.find("+CSQ: ") + 5, o.find(",") - o.find("+CSQ: ") - 5).c_str());
        }
        //std::cout << "networkQuality: " << networkQuality << std::endl;
    }

    void getNetworkQuality()
    {
        requests.push_back({&GSM::updateNetworkQuality, priority::normal});
    }

    void updateFlightMode()
    {
        if(flightMode)
        {
            std::cout << "Flight Mode ON" << std::endl;
            std::cout << send("AT+CFUN=4", "AT+CFUN", 1000) << std::endl;
        }
        else
        {
            std::cout << "Flight Mode OFF" << std::endl;
            //std::cout << send("AT+CFUN=6", "AT+CFUN", 1000) << std::endl;
            std::cout << send("AT+CFUN=1", "AT+CFUN", 1000) << std::endl;
        }
    }

    bool isFlightMode()
    {
        return flightMode;
    }

    void setFlightMode(bool mode)
    {
        eventHandlerGsm.setTimeout(new Callback<>([mode](){ appendRequest({updateFlightMode}); }), 0);
        flightMode = mode;
    }

    void run()
    {
        init();

        //PaxOS_Delay(50000);

        requests.push_back({[](){ send("AT+CNTP=\"time.google.com\",8", "AT+CNTP"); send("AT+CNTP","AT+CNTP", 1000); }, priority::high});

        updateHour();
        getNetworkQuality();
        onMessage();
        getVoltage();

        eventHandlerGsm.setInterval(&GSM::getHour, 5000);
        eventHandlerGsm.setInterval(&GSM::getNetworkQuality, 10000);
        eventHandlerGsm.setInterval([](){ requests.push_back({&GSM::getVoltage, GSM::priority::normal}); }, 5000);
        eventHandlerGsm.setInterval([](){ requests.push_back({&GSM::onMessage, GSM::priority::normal}); }, 5000);

        keys.push_back({"RING", &GSM::onRinging});
        keys.push_back({"+CMTI:", &GSM::onMessage});
        keys.push_back({"VOICE CALL: END", &GSM::onHangOff});
        keys.push_back({"VOICE CALL: BEGIN", [](){ state.callState = CallState::CALLING; }});

        while (true)
        {
            PaxOS_Delay(5);

            eventHandlerGsm.update();

            download();

            process();
            data = "";

            checkRequest();
        }
    }
};
