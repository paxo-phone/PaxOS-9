#include "pdu.hpp"

#include <bitset>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>

std::string convert_semi_octet(const std::string& s)
{
    std::string o;
    for (uint32_t i = 0; i < s.size(); i += 2)
    {
        o += s[i + 1];
        o += s[i];
    }

    return o;
}

std::string decodeGSM7bit(const std::string& encoded)
{
    std::string decoded;
    int length = encoded.length() / 2;
    int bitOffset = 0;

    for (int i = 0; i < length; ++i)
    {
        int byteIndex = (i * 7) / 8;
        int shift = bitOffset % 8;

        uint8_t currentByte = std::stoi(encoded.substr(byteIndex * 2, 2), nullptr, 16);
        uint8_t nextByte = (byteIndex + 1 < encoded.length() / 2)
                               ? std::stoi(encoded.substr((byteIndex + 1) * 2, 2), nullptr, 16)
                               : 0;

        uint8_t septet = ((currentByte >> shift) | (nextByte << (8 - shift))) & 0x7F;

        decoded += static_cast<char>(septet);
        bitOffset += 7;
    }

    return decoded;
}

int hex_to_int(const std::string& s)
{
    int result = 0;
    for (char c : s)
    {
        result *= 16;
        if (c >= '0' && c <= '9')
            result += c - '0';
        else if (c >= 'A' && c <= 'F')
            result += c - 'A' + 10;
        else if (c >= 'a' && c <= 'f')
            result += c - 'a' + 10;
    }
    return result;
}

int hexCharToInt(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    throw std::invalid_argument("Invalid hex character");
}

// Function to convert a Latin-1 encoded hexadecimal string to a UTF-8 string
std::string latin1HexToUtf8(const std::string& hexString)
{
    std::ostringstream utf8Stream;

    for (size_t i = 0; i < hexString.length(); i += 2)
    {
        // Convert the two hex characters to a byte
        char byte =
            static_cast<char>((hexCharToInt(hexString[i]) << 4) | hexCharToInt(hexString[i + 1]));

        // Convert the byte from Latin-1 to UTF-8
        unsigned char ubyte = static_cast<unsigned char>(byte);
        if (ubyte < 0x80)
        {
            // 1-byte UTF-8 (ASCII)
            utf8Stream << byte;
        }
        else
        {
            // 2-byte UTF-8
            utf8Stream << static_cast<char>(0xC0 | (ubyte >> 6));
            utf8Stream << static_cast<char>(0x80 | (ubyte & 0x3F));
        }
    }

    return utf8Stream.str();
}

std::string hex_to_text(const std::string& s)
{
    std::string result;
    for (size_t i = 0; i < s.length(); i += 2)
    {
        char c = static_cast<char>(hex_to_int(s.substr(i, 2)));
        if (c >= 32 && c < 127)
            result += c;
        else
            result += '~';
    }
    return result;
}

bool getBit(uint8_t byte, uint8_t bit)
{
    return (byte & (1 << bit)) != 0;
}

PDU decodePDU(std::string pdu)
{
    std::string number;
    std::string text;
    std::string url;

    int i = 0; // index in pdu

    int SMSC_length = hex_to_int(pdu.substr(i, 2)) - 2;
    i += 2;
    int SMSC_number_type = hex_to_int(pdu.substr(i, 2));
    i += 2;
    std::string SMSC = convert_semi_octet(pdu.substr(i, SMSC_length * 2));
    i += SMSC_length * 2;
    i += 2; // skip F0

    // std::cout << "SMSC_length: " << SMSC_length << std::endl;
    // std::cout << "SMSC_number_type: " << SMSC_number_type << std::endl;
    // std::cout << "SMSC: " << SMSC << std::endl;

    // std::cout << "PDU_mode: " << pdu.substr(i, 2) << std::endl;
    char PDU_mode = hex_to_int(pdu.substr(i, 2)) - 2;
    i += 2;

    // std::cout << "PDU_mode: " << int(PDU_mode) << " " << std::bitset<8>(PDU_mode).to_string() <<
    // std::endl;

    /*if(getBit(PDU_mode, 7))
        std::cout << "TP-RP enabled" << std::endl;

    if(getBit(PDU_mode, 6))
        std::cout << "TP-UDHI enabled" << std::endl;

    if(getBit(PDU_mode, 5))
        std::cout << "TP-SRI enabled" << std::endl;

    if(getBit(PDU_mode, 2))
        std::cout << "TP-MMS enabled" << std::endl;*/

    // std::cout << "TP-MTI: " << (int) getBit(PDU_mode, 0) << (int) getBit(PDU_mode, 1) <<
    // std::endl;

    int Adress_length = hex_to_int(pdu.substr(i, 2)) + 1;
    i += 2;
    int Adress_number_type = hex_to_int(pdu.substr(i, 2));
    i += 2;
    std::string Adress = convert_semi_octet(pdu.substr(i, Adress_length));
    i += Adress_length;
    Adress = Adress.substr(0, Adress.find('F'));

    // std::cout << "Adress_length: " << Adress_length << std::endl;
    // std::cout << "Adress_number_type: " << Adress_number_type << std::endl;
    // std::cout << "Adress: " << Adress << std::endl;

    char PID = hex_to_int(pdu.substr(i, 2));
    i += 2;
    char DSC = hex_to_int(pdu.substr(i, 2));
    i += 2;

    PDU_type mode = UNKNOWN;
    bool is_unicode = false;

    if (getBit(DSC, 3) == 0 && getBit(DSC, 2) == 0)
    {
        // std::cout << "SMS mode" << std::endl;
        mode = SMS;
    }
    else if (getBit(DSC, 3) == 0 && getBit(DSC, 2) == 1)
    {
        // std::cout << "MMS mode" << std::endl;
        mode = MMS;
    }
    else if (getBit(DSC, 3) == 1 && getBit(DSC, 2) == 0)
    {
        is_unicode = true;
        mode = SMS;
        // std::cout << "SMS mode with unicode" << std::endl;
    }
    else
    {
        // std::cout << "Unknown mode" << std::endl;
    }
    // std::cout << "DSC: " << (int) getBit(DSC, 3) << (int) getBit(DSC, 2) << "  " <<
    // std::bitset<8>(DSC).to_string() << std::endl;

    i += 7 * 2; // timestamp

    int Message_length = hex_to_int(pdu.substr(i, 2)) + 1;
    i += 2;
    std::string Message = pdu.substr(i, Message_length * 2);
    i += Message_length * 2;

    if (mode == SMS)
    {
        number = "+" + Adress;

        if (is_unicode)
        {
            for (int j = 0; j < Message.length(); j += 2)
                if (Message.substr(j, 2) == "00")
                    Message = Message.substr(0, j) + Message.substr(j + 2);

            // std::cout << "Message unicode: " << Message << std::endl;

            text = latin1HexToUtf8(Message);
        }
        else
            text = decodeGSM7bit(Message);
    }
    else
    {
        Message = hex_to_text(Message);

        number = Message.substr(Message.find("+"), Message.find("/") - Message.find("+"));
        url = Message.substr(
            Message.find("http"),
            Message.find("~", Message.find("http")) - Message.find("http")
        );
    }

    std::cout << "Number: " << number << std::endl;
    std::cout << "Text: " << text << std::endl;
    std::cout << "URL: " << url << std::endl;

    return {number, text, url, mode};
}
