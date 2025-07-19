#include "gsm2.hpp" // Assuming gsm2.hpp is the header file name

#include <algorithm>
#include <atomic> // For thread-safe flags if needed
#include <chrono>
#include <cmath>     // For ceil
#include <cstdio>    // For sscanf
#include <delay.hpp> // Assumes PaxOS_Delay
#include <functional>
#include <iomanip> // For std::setw, std::setfill
#include <iostream>
#include <memory>
#include <mutex> // For thread safety
#include <sstream>
#include <standby.hpp> // Assumes StandbyMode
#include <stdexcept>   // For exceptions in helpers (optional)
#include <string>
#include <threads.hpp> // Assumes eventHandlerBack
#include <vector>

#ifdef ESP_PLATFORM
#include <Arduino.h>

#ifndef RX
#define RX 26
#endif

#ifndef TX
#define TX 27
#endif

#ifndef GSM_PWR_PIN
#define GSM_PWR_PIN 32
#endif
#endif

#include <contacts.hpp>
#include <conversation.hpp>
#include <filestream.hpp>

const char* daysOfWeek[7] =
    {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
const char* daysOfMonth[12] = {
    "Janvier",
    "Fevrier",
    "Mars",
    "Avril",
    "Mai",
    "Juin",
    "Juillet",
    "Aout",
    "Septembre",
    "Octobre",
    "Novembre",
    "Decembre"
};

inline long long getCurrentTimestamp()
{
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch()
    )
        .count();
}

// --- END: User-Provided External Dependencies (Declarations) ---

namespace Gsm
{
    // --- Internal State Variables ---

    EventHandler eventHandlerGsm;

    // Network Quality
    static int currentRssi = 99;
    static int currentBer = 99;
    static bool networkQualityValid = false;
    static std::chrono::steady_clock::time_point lastQualityUpdateTime;

    // Connection Status (GPRS Attachment)
    static bool isGprsAttached = false;
    static bool gprsStateValid = false;
    static std::chrono::steady_clock::time_point lastGprsUpdateTime;

    // Flight Mode
    static bool flightModeState = false; // Based on AT+CFUN? result (1=ON, 4=OFF)
    static bool flightModeStateValid = false;
    static std::chrono::steady_clock::time_point lastFlightModeUpdateTime;

    // Voltage
    static int currentVoltage_mV = -1;
    std::vector<float> battery_voltage_history;
    static bool voltageValid = false;
    static std::chrono::steady_clock::time_point lastVoltageUpdateTime;

    // PIN Status
    static bool pinRequiresPin = true; // Assume PIN needed until checked
    static bool pinStatusValid = false;
    static std::chrono::steady_clock::time_point lastPinStatusUpdateTime;

    // PDU Mode
    static bool pduModeEnabled = false; // Assume Text mode until checked
    static bool pduModeStateValid = false;
    static std::chrono::steady_clock::time_point lastPduModeUpdateTime;

    // Call State & Info
    static CallState currentCallState = CallState::UNKNOWN;
    static std::string lastIncomingCallNumber = "";

    // Request Queue & Current Request (Internal)
    static std::vector<std::shared_ptr<Request>> requests;
    static std::shared_ptr<Request> currentRequest = nullptr;

    static std::mutex requestMutex; // Mutex for thread-safe request handling

    namespace ExternalEvents
    {
        std::function<void(void)> onIncommingCall;
        std::function<void(void)> onNewMessage;
        std::function<void(void)> onNewMessageError;
    } // namespace ExternalEvents

    // --- Time Sub-Namespace Implementation ---
    namespace Time
    {
        // Internal state for time
        static int year = -1;
        static int month = -1;
        static int day = -1;
        static int hour = -1;
        static int minute = -1;
        static int second = -1;
        static int timezoneOffsetQuarterHours = 0; // e.g., +4 for UTC+1
        static bool timeValid = false;
        static std::chrono::steady_clock::time_point lastTimeUpdateTime;

        // Enqueues a request to check AT+CCLK?
        static void updateTimeInternal()
        {
            auto request = std::make_shared<Request>();
            request->command = "AT+CCLK?";
            request->callback = [](const std::string& response) -> bool
            {
                int l_yy, l_MM, l_dd, l_hh, l_mm, l_ss, l_zz;
                char sign;
                bool parsed_ok = false;

                size_t start_pos = response.find("+CCLK:");
                if (start_pos != std::string::npos)
                {
                    size_t quote_start = response.find('"', start_pos);
                    size_t quote_end = std::string::npos;
                    if (quote_start != std::string::npos)
                        quote_end = response.find('"', quote_start + 1);

                    if (quote_start != std::string::npos && quote_end != std::string::npos)
                    {
                        std::string time_str =
                            response.substr(quote_start + 1, quote_end - quote_start - 1);
                        int fields_read = sscanf(
                            time_str.c_str(),
                            "%d/%d/%d,%d:%d:%d%c%d",
                            &l_yy,
                            &l_MM,
                            &l_dd,
                            &l_hh,
                            &l_mm,
                            &l_ss,
                            &sign,
                            &l_zz
                        );

                        if (fields_read == 8)
                        {
                            if (l_yy >= 0 && l_yy <= 99 && l_MM >= 1 && l_MM <= 12 && l_dd >= 1 &&
                                l_dd <= 31 && l_hh >= 0 && l_hh <= 23 && l_mm >= 0 && l_mm <= 59 &&
                                l_ss >= 0 && l_ss <= 59 && (sign == '+' || sign == '-'))
                            {
                                year = 2000 + l_yy;
                                month = l_MM;
                                day = l_dd;
                                hour = l_hh;
                                minute = l_mm;
                                second = l_ss;
                                timezoneOffsetQuarterHours = (sign == '+') ? l_zz : -l_zz;
                                timeValid = true;
                                lastTimeUpdateTime = std::chrono::steady_clock::now();
                                parsed_ok = true;
                            }
                        }
                    }
                }

                if (!parsed_ok)
                {
                }
                return false;
            };
            std::lock_guard<std::mutex> lock(requestMutex);
            requests.push_back(request);
        }

        void syncNetworkTime()
        {
            updateTimeInternal();
        }

        int getYear()
        {
            return timeValid ? year : -1;
        }

        int getMonth()
        {
            return timeValid ? month : -1;
        }

        int getDay()
        {
            return timeValid ? day : -1;
        }

        int getHour()
        {
            return timeValid ? hour : -1;
        }

        int getMinute()
        {
            return timeValid ? minute : -1;
        }

        int getSecond()
        {
            return timeValid ? second : -1;
        }

        int getTimezoneOffsetQuarterHours()
        {
            return timeValid ? timezoneOffsetQuarterHours : 0;
        }

        bool isTimeValid()
        {
            return timeValid;
        }

    } // namespace Time

    // --- PDU Decoding Structures and Enums (Internal) ---
    enum class SmsType
    {
        TEXT_SMS,
        MMS_NOTIFICATION,
        STATUS_REPORT,
        UNKNOWN
    };

    struct DecodedPdu
    {
        SmsType type = SmsType::UNKNOWN;
        std::string senderNumber;
        std::string messageContent;
        std::string serviceCenterTimestamp; // Format: "YYMMDDHHMMSS+/-ZZ"
        std::string mmsUrl;                 // If MMS_NOTIFICATION and URL is found
        bool hasUserDataHeader = false;
    };

    // --- PDU Decoding Helper Functions (Internal) ---
    static unsigned char hexPairToByte(char c1, char c2)
    {
        unsigned char byte = 0;
        auto charToVal = [](char c) -> unsigned char
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;
            if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;
            throw std::invalid_argument("Invalid hex char");
        };
        byte = (charToVal(c1) << 4) | charToVal(c2);
        return byte;
    }

    static std::vector<unsigned char> hexStringToBytes(const std::string& hex)
    {
        if (hex.length() % 2 != 0)
            throw std::invalid_argument("Hex string must have an even length.");
        std::vector<unsigned char> bytes;
        for (size_t i = 0; i < hex.length(); i += 2)
            bytes.push_back(hexPairToByte(hex[i], hex[i + 1]));
        return bytes;
    }

    static std::string decodeSemiOctets(const std::string& data_hex, int num_digits_to_read)
    {
        std::string result = "";
        for (int i = 0; i < num_digits_to_read; ++i)
        {
            int byte_idx = i / 2;
            char digit_char;
            if (i % 2 == 0)
                digit_char = data_hex[byte_idx * 2 + 1];
            else
                digit_char = data_hex[byte_idx * 2];
            if (digit_char == 'F' || digit_char == 'f')
            {
                if (i == num_digits_to_read - 1 && (num_digits_to_read % 2 != 0))
                    continue;
            }
            result += digit_char;
        }
        return result;
    }

    static std::string gsm7SeptetToUtf8(unsigned char septet)
    {
        septet &= 0x7F; // Ensure only 7 bits are considered
        switch (septet)
        {
        case 0x00:
            return "@";
        case 0x01:
            return "\xC2\xA3"; /* £ */
        case 0x02:
            return "$";
        case 0x03:
            return "\xC2\xA5"; /* ¥ */
        case 0x04:
            return "\xC3\xA8"; /* è */
        case 0x05:
            return "\xC3\xA9"; /* é */
        case 0x06:
            return "\xC3\xB9"; /* ù */
        case 0x07:
            return "\xC3\xAC"; /* ì */
        case 0x08:
            return "\xC3\xB2"; /* ò */
        case 0x09:
            return "\xC3\x87"; /* Ç */
        case 0x0A:
            return "\n";
        case 0x0B:
            return "\xC3\x98"; /* Ø */
        case 0x0C:
            return "\xC3\xB8"; /* ø */
        case 0x0D:
            return "\r";
        case 0x0E:
            return "\xC3\x85"; /* Å */
        case 0x0F:
            return "\xC3\xA5"; /* å */
        case 0x10:
            return "\xCE\x94"; /* Δ */
        case 0x11:
            return "_";
        case 0x12:
            return "\xCE\xA6"; /* Φ */
        case 0x13:
            return "\xCE\x93"; /* Γ */
        case 0x14:
            return "\xCE\x9B"; /* Λ */
        case 0x15:
            return "\xCE\xA9"; /* Ω */
        case 0x16:
            return "\xCE\xA0"; /* Π */
        case 0x17:
            return "\xCE\xA8"; /* Ψ */
        case 0x18:
            return "\xCE\xA3"; /* Σ */
        case 0x19:
            return "\xCE\x98"; /* Θ */
        case 0x1A:
            return "\xCE\x9E"; /* Ξ */
        case 0x1B:
            return " ";
        case 0x1C:
            return "\xC3\x86"; /* Æ */
        case 0x1D:
            return "\xC3\xA6"; /* æ */
        case 0x1E:
            return "\xC3\x9F"; /* ß */
        case 0x1F:
            return "\xC3\x89"; /* É */
        case 0x20:
            return " ";
        case 0x21:
            return "!";
        case 0x22:
            return "\"";
        case 0x23:
            return "#";
        case 0x25:
            return "%";
        case 0x26:
            return "&";
        case 0x27:
            return "'";
        case 0x28:
            return "(";
        case 0x29:
            return ")";
        case 0x2A:
            return "*";
        case 0x2B:
            return "+";
        case 0x2C:
            return ",";
        case 0x2D:
            return "-";
        case 0x2E:
            return ".";
        case 0x2F:
            return "/";
        case 0x30:
            return "0";
        case 0x31:
            return "1";
        case 0x32:
            return "2";
        case 0x33:
            return "3";
        case 0x34:
            return "4";
        case 0x35:
            return "5";
        case 0x36:
            return "6";
        case 0x37:
            return "7";
        case 0x38:
            return "8";
        case 0x39:
            return "9";
        case 0x3A:
            return ":";
        case 0x3B:
            return ";";
        case 0x3C:
            return "<";
        case 0x3D:
            return "=";
        case 0x3E:
            return ">";
        case 0x3F:
            return "?";
        case 0x40:
            return "@";
        case 0x41:
            return "A";
        case 0x42:
            return "B";
        case 0x43:
            return "C";
        case 0x44:
            return "D";
        case 0x45:
            return "E";
        case 0x46:
            return "F";
        case 0x47:
            return "G";
        case 0x48:
            return "H";
        case 0x49:
            return "I";
        case 0x4A:
            return "J";
        case 0x4B:
            return "K";
        case 0x4C:
            return "L";
        case 0x4D:
            return "M";
        case 0x4E:
            return "N";
        case 0x4F:
            return "O";
        case 0x50:
            return "P";
        case 0x51:
            return "Q";
        case 0x52:
            return "R";
        case 0x53:
            return "S";
        case 0x54:
            return "T";
        case 0x55:
            return "U";
        case 0x56:
            return "V";
        case 0x57:
            return "W";
        case 0x58:
            return "X";
        case 0x59:
            return "Y";
        case 0x5A:
            return "Z";
        case 0x5B:
            return "\xC3\xA4"; /* ä */
        case 0x5C:
            return "\xC3\xB6"; /* ö */
        case 0x5D:
            return "\xC3\xBC"; /* ü */
        case 0x5E:
            return "\xC3\x84"; /* Ä */
        case 0x5F:
            return "_";
        case 0x60:
            return "\xC3\x9F";
        case 0x61:
            return "a";
        case 0x62:
            return "b";
        case 0x63:
            return "c";
        case 0x64:
            return "d";
        case 0x65:
            return "e";
        case 0x66:
            return "f";
        case 0x67:
            return "g";
        case 0x68:
            return "h";
        case 0x69:
            return "i";
        case 0x6A:
            return "j";
        case 0x6B:
            return "k";
        case 0x6C:
            return "l";
        case 0x6D:
            return "m";
        case 0x6E:
            return "n";
        case 0x6F:
            return "o";
        case 0x70:
            return "p";
        case 0x71:
            return "q";
        case 0x72:
            return "r";
        case 0x73:
            return "s";
        case 0x74:
            return "t";
        case 0x75:
            return "u";
        case 0x76:
            return "v";
        case 0x77:
            return "w";
        case 0x78:
            return "x";
        case 0x79:
            return "y";
        case 0x7A:
            return "z";
        case 0x7B:
            return "\xC3\xA4";
        case 0x7C:
            return "\xC3\xB6";
        case 0x7D:
            return "\xC3\xBC";
        case 0x7E:
            return "a";
        case 0x7F:
            return "\xE2\x82\xAC"; /* € */
        default:
            return "?";
        }
    }

    static char gsm7SeptetToChar(unsigned char septet)
    {
        switch (septet)
        {
        case 0x00:
            return '@';
        case 0x01:
            return '\xA3';
        case 0x02:
            return '$';
        case 0x03:
            return '\xA5';
        case 0x04:
            return '\xE8';
        case 0x05:
            return '\xE9';
        case 0x06:
            return '\xF9';
        case 0x07:
            return '\xEC';
        case 0x08:
            return '\xF2';
        case 0x09:
            return '\xC7';
        case 0x0A:
            return '\n';
        case 0x0B:
            return '\xD8';
        case 0x0C:
            return '\xF8';
        case 0x0D:
            return '\r';
        case 0x0E:
            return '\xC5';
        case 0x0F:
            return '\xE5';
        case 0x10:
            return '?';
        case 0x11:
            return '_';
        case 0x12:
            return '?';
        case 0x13:
            return '?';
        case 0x14:
            return '?';
        case 0x15:
            return '?';
        case 0x16:
            return '?';
        case 0x17:
            return '?';
        case 0x18:
            return '?';
        case 0x19:
            return '?';
        case 0x1A:
            return '?';
        case 0x1B:
            return ' ';
        case 0x1C:
            return '\xC6';
        case 0x1D:
            return '\xE6';
        case 0x1E:
            return '\xDF';
        case 0x1F:
            return '\xC9';
        case 0x20:
            return ' ';
        default:
            if ((septet >= '0' && septet <= '9') || (septet >= 'A' && septet <= 'Z') ||
                (septet >= 'a' && septet <= 'z') ||
                std::string(" !\"#%&'()*+,-./:;<=>?").find(static_cast<char>(septet)) !=
                    std::string::npos)
            {
                return static_cast<char>(septet);
            }
            return '?';
        }
    }

    static std::string unpack7BitData(
        const std::vector<unsigned char>& packed_octets, int num_septets, int udh_septet_length = 0
    )
    {
        std::string text = "";
        if (num_septets <= 0 || packed_octets.empty())
            return text;

        int current_octet_idx = 0;
        int bit_shift = 0;

        for (int i = 0; i < num_septets; ++i)
        {
            if (current_octet_idx >= packed_octets.size())
                break;
            unsigned char current_septet;

            current_septet = (packed_octets[current_octet_idx] >> bit_shift);

            if (bit_shift > 0)
            {
                if (current_octet_idx + 1 < packed_octets.size())
                {
                    current_septet |= (packed_octets[current_octet_idx + 1] << (8 - bit_shift));
                }
                else if (i < num_septets - 1 ||
                         ((i == num_septets - 1) && (bit_shift + (8 - bit_shift) < 7) &&
                          packed_octets.size() * 8 / 7 < num_septets))
                {
                    if (current_octet_idx == packed_octets.size() - 1)
                    {
                    }
                }
            }

            current_septet &= 0x7F;

            if (i >= udh_septet_length)
                text += gsm7SeptetToUtf8(current_septet);

            bit_shift += 7;
            if (bit_shift >= 8)
            {
                current_octet_idx++;
                bit_shift -= 8;
            }
        }
        return text;
    }

    static std::string ucs2BytesToUtf8(const std::vector<unsigned char>& ucs2_bytes, int offset)
    {
        std::string utf8_text;
        for (size_t i = offset; i < ucs2_bytes.size(); i += 2)
        {
            if (i + 1 >= ucs2_bytes.size())
            {
                utf8_text += "?";
                break;
            }

            unsigned short ucs2_code_unit = (static_cast<unsigned short>(ucs2_bytes[i]) << 8) |
                                            static_cast<unsigned short>(ucs2_bytes[i + 1]);

            if (ucs2_code_unit <= 0x7F)
            {
                utf8_text += static_cast<char>(ucs2_code_unit);
            }
            else if (ucs2_code_unit <= 0x7FF)
            {
                utf8_text += static_cast<char>(0xC0 | (ucs2_code_unit >> 6));
                utf8_text += static_cast<char>(0x80 | (ucs2_code_unit & 0x3F));
            }
            else if (ucs2_code_unit >= 0xD800 && ucs2_code_unit <= 0xDFFF)
            {
                utf8_text += "?";
            }
            else
            {
                utf8_text += static_cast<char>(0xE0 | (ucs2_code_unit >> 12));
                utf8_text += static_cast<char>(0x80 | ((ucs2_code_unit >> 6) & 0x3F));
                utf8_text += static_cast<char>(0x80 | (ucs2_code_unit & 0x3F));
            }
        }
        return utf8_text;
    }

    static bool decodePduDeliver(const std::string& pdu_hex_string, DecodedPdu& result)
    {
        try
        {
            int current_pos = 0;
            int sca_len_octets =
                hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos + 1]);
            current_pos += 2;
            if (sca_len_octets > 0)
            {
                if (current_pos + sca_len_octets * 2 > pdu_hex_string.length())
                    return false;
                current_pos += sca_len_octets * 2;
            }

            if (current_pos + 2 > pdu_hex_string.length())
                return false;
            unsigned char tpdu_type_octet =
                hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos + 1]);
            current_pos += 2;
            int tp_mti = tpdu_type_octet & 0x03;
            result.hasUserDataHeader = (tpdu_type_octet & 0x40) != 0;

            if (tp_mti == 0x00)
            {
                result.type = SmsType::TEXT_SMS;

                if (current_pos + 2 > pdu_hex_string.length())
                    return false;
                int oa_len_digits =
                    hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos + 1]);
                current_pos += 2;
                if (current_pos + 2 > pdu_hex_string.length())
                    return false;
                current_pos += 2;
                int oa_addr_len_octets = (oa_len_digits + 1) / 2;
                if (current_pos + oa_addr_len_octets * 2 > pdu_hex_string.length())
                    return false;
                std::string oa_hex = pdu_hex_string.substr(current_pos, oa_addr_len_octets * 2);
                result.senderNumber = decodeSemiOctets(oa_hex, oa_len_digits);
                current_pos += oa_addr_len_octets * 2;

                if (current_pos + 2 > pdu_hex_string.length())
                    return false;
                current_pos += 2;

                if (current_pos + 2 > pdu_hex_string.length())
                    return false;
                unsigned char tp_dcs =
                    hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos + 1]);
                current_pos += 2;

                bool is_7bit_encoding = false;
                bool is_8bit_encoding = false;
                bool is_ucs2_encoding = false;
                unsigned char dcs_group = (tp_dcs & 0xC0);
                unsigned char dcs_char_set = (tp_dcs & 0x0C) >> 2;

                if (dcs_group == 0x00)
                {
                    if ((tp_dcs & 0x20) == 0x00)
                    {
                        if (dcs_char_set == 0x00)
                            is_7bit_encoding = true;
                        else if (dcs_char_set == 0x01)
                            is_8bit_encoding = true;
                        else if (dcs_char_set == 0x02)
                            is_ucs2_encoding = true;
                    }
                    else
                    {
                        is_7bit_encoding = true;
                    }
                }
                else if (dcs_group == 0x40)
                {
                    is_8bit_encoding = true;
                }
                else if (dcs_group == 0x80)
                {
                    return false;
                }
                else if (dcs_group == 0xC0)
                {
                    if ((tp_dcs & 0x20) == 0x00)
                    {
                        if (dcs_char_set == 0x00)
                            is_7bit_encoding = true;
                        else if (dcs_char_set == 0x01)
                            is_8bit_encoding = true;
                        else if (dcs_char_set == 0x02)
                            is_ucs2_encoding = true;
                    }
                    else
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }

                if (current_pos + 7 * 2 > pdu_hex_string.length())
                    return false;
                std::string scts_hex = pdu_hex_string.substr(current_pos, 7 * 2);
                result.serviceCenterTimestamp = decodeSemiOctets(scts_hex, 14);
                current_pos += 7 * 2;

                if (current_pos + 2 > pdu_hex_string.length())
                    return false;
                unsigned char tp_udl =
                    hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos + 1]);
                current_pos += 2;

                int ud_expected_octet_len = 0;
                if (is_7bit_encoding)
                    ud_expected_octet_len = static_cast<int>(std::ceil(tp_udl * 7.0 / 8.0));
                else if (is_8bit_encoding || is_ucs2_encoding)
                    ud_expected_octet_len = tp_udl;
                else
                    return false;

                if (current_pos + ud_expected_octet_len * 2 > pdu_hex_string.length())
                {
                    ud_expected_octet_len = (pdu_hex_string.length() - current_pos) / 2;
                    if (ud_expected_octet_len < 0)
                        ud_expected_octet_len = 0;
                    if (ud_expected_octet_len == 0)
                    {
                        result.messageContent = "";
                        return true;
                    }
                }
                std::string ud_hex = pdu_hex_string.substr(current_pos, ud_expected_octet_len * 2);
                std::vector<unsigned char> ud_bytes = hexStringToBytes(ud_hex);

                int udh_octet_len = 0;
                int udh_septet_len = 0;
                if (result.hasUserDataHeader && !ud_bytes.empty())
                {
                    udh_octet_len = ud_bytes[0] + 1;
                    if (udh_octet_len > ud_bytes.size())
                    {
                        udh_octet_len = 0;
                        result.hasUserDataHeader = false;
                    }
                    else
                    {
                        if (udh_octet_len >= 6)
                        {
                            for (int ie_pos = 1; ie_pos < udh_octet_len;)
                            {
                                if (ie_pos + 1 >= udh_octet_len)
                                    break;
                                unsigned char iei = ud_bytes[ie_pos];
                                unsigned char iedl = ud_bytes[ie_pos + 1];
                                if (ie_pos + 1 + iedl >= udh_octet_len)
                                    break;

                                if (iei == 0x05 && iedl == 0x04)
                                {
                                    unsigned short dest_port =
                                        (ud_bytes[ie_pos + 2] << 8) | ud_bytes[ie_pos + 3];
                                    if (dest_port == 2948 || dest_port == 9200 ||
                                        dest_port == 9201 || dest_port == 9202 || dest_port == 9203)
                                    {
                                        result.type = SmsType::MMS_NOTIFICATION;
                                        break;
                                    }
                                }
                                ie_pos += 2 + iedl;
                            }
                        }

                        if (is_7bit_encoding)
                            udh_septet_len = static_cast<int>(std::ceil(udh_octet_len * 8.0 / 7.0));
                    }
                }

                if (is_7bit_encoding)
                {
                    result.messageContent = unpack7BitData(
                        ud_bytes,
                        tp_udl,
                        result.hasUserDataHeader ? udh_septet_len : 0
                    );
                }
                else if (is_8bit_encoding)
                {
                    std::string msg_data_8bit;
                    for (size_t k = udh_octet_len; k < ud_bytes.size(); ++k)
                        msg_data_8bit += static_cast<char>(ud_bytes[k]);
                    result.messageContent = msg_data_8bit;
                }
                else if (is_ucs2_encoding)
                {
                    if ((ud_bytes.size() - udh_octet_len) % 2 != 0)
                    {
                    }
                    result.messageContent = ucs2BytesToUtf8(ud_bytes, udh_octet_len);
                }
                else
                {
                    return false;
                }

                if (result.type == SmsType::MMS_NOTIFICATION)
                {
                    size_t http_pos = result.messageContent.find("http");
                    if (http_pos != std::string::npos)
                    {
                        size_t url_end = result.messageContent.find_first_of(" \t\r\n\0", http_pos);
                        if (url_end == std::string::npos)
                        {
                            result.mmsUrl = result.messageContent.substr(http_pos);
                        }
                        else
                        {
                            result.mmsUrl =
                                result.messageContent.substr(http_pos, url_end - http_pos);
                        }
                    }
                }
            }
            else if (tp_mti == 0x02)
            {
                result.type = SmsType::STATUS_REPORT;
                if (current_pos + 7 * 2 + 2 > pdu_hex_string.length())
                {
                }
                return true;
            }
            else
            {
                return false;
            }

            return true;
        }
        catch (const std::exception& e)
        {
            return false;
        }
        catch (...)
        {
            return false;
        }
    }

    static void processAndStoreSms(const std::string& pdu_hex_string, int message_idx)
    {
        DecodedPdu decoded_sms;
        if (decodePduDeliver(pdu_hex_string, decoded_sms))
        {

            if (decoded_sms.type == SmsType::TEXT_SMS ||
                decoded_sms.type == SmsType::MMS_NOTIFICATION)
            {
                if (decoded_sms.type == SmsType::MMS_NOTIFICATION)
                {
                    if (!decoded_sms.mmsUrl.empty())
                    {
                    }
                }
                else
                {
                    auto contact = Contacts::getByNumber("+" + decoded_sms.senderNumber);

                    Conversations::Conversation conv;
                    storage::Path convPath(
                        std::string(MESSAGES_LOCATION) + "/+" + decoded_sms.senderNumber + ".json"
                    );
                    if (convPath.exists())
                        Conversations::loadConversation(convPath, conv);
                    else
                        conv.number = decoded_sms.senderNumber;

                    conv.messages.push_back(
                        {decoded_sms.messageContent, true, std::to_string(getCurrentTimestamp())}
                    );
                    Conversations::saveConversation(convPath, conv);

                    storage::FileStream file(
                        std::string(MESSAGES_NOTIF_LOCATION),
                        storage::Mode::READ
                    );
                    std::string content = file.read();
                    file.close();

                    if (content.find("+" + decoded_sms.senderNumber) == std::string::npos)
                    {
                        storage::FileStream file2(
                            storage::Path(std::string(MESSAGES_NOTIF_LOCATION)).str(),
                            storage::Mode::APPEND
                        );
                        file2.write("+" + decoded_sms.senderNumber + "\n");
                        file2.close();
                    }
                }
                if (ExternalEvents::onNewMessage)
                    ExternalEvents::onNewMessage();
            }

            auto delete_req = std::make_shared<Request>();
            delete_req->command = "AT+CMGD=" + std::to_string(message_idx);
            delete_req->callback = [message_idx](const std::string& response) -> bool
            {
                if (response.find("OK") != std::string::npos)
                {
                }
                else
                {
                }
                return false;
            };
            std::lock_guard<std::mutex> lock(requestMutex);
            requests.push_back(delete_req);
        }
        else
        {
        }
    }

    // Forward declarations for helper functions
    bool isEndIdentifier(const std::string& data);
    bool isURC(const std::string& data);

    // end of forward declarations

    static void queueReadSms(const std::string& memory_store, int index)
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CMGR=" + std::to_string(index);
        request->callback = [index](const std::string& response_block) -> bool
        {
            std::string pdu_line;
            bool found_pdu = false;
            std::stringstream ss_block(response_block);
            std::string line;
            bool cmgr_header_found = false;

            while (std::getline(ss_block, line, '\n'))
            {
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);
                if (line.empty())
                    continue;

                if (line.rfind("+CMGR:", 0) == 0)
                {
                    cmgr_header_found = true;
                }
                else if (cmgr_header_found && line != "OK" && line.rfind("ERROR", 0) != 0 &&
                         !isEndIdentifier(line) && !isURC(line))
                {
                    pdu_line = line;
                    found_pdu = true;
                    break;
                }
            }

            if (found_pdu)
            {
                processAndStoreSms(pdu_line, index);
            }
            else
            {
            }
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    void checkForMessages()
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CMGL=0";

        request->callback = [](const std::string& response_block) -> bool
        {
            std::stringstream ss_block(response_block);
            std::string line;
            int messages_found_and_processed = 0;
            bool command_ok = response_block.find("OK") != std::string::npos;
            bool command_error = response_block.find("ERROR") != std::string::npos;

            int current_message_idx = -1;
            bool expect_pdu_next = false;

            while (std::getline(ss_block, line, '\n'))
            {
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);
                if (line.empty())
                    continue;

                if (line.rfind("+CMGL:", 0) == 0)
                {
                    std::string data_part = line.substr(6);
                    size_t first_comma = data_part.find(',');
                    if (first_comma != std::string::npos)
                    {
                        std::string index_str = data_part.substr(0, first_comma);
                        index_str.erase(0, index_str.find_first_not_of(" \t"));
                        index_str.erase(index_str.find_last_not_of(" \t") + 1);
                        try
                        {
                            current_message_idx = std::stoi(index_str);
                            expect_pdu_next = true;
                        }
                        catch (const std::invalid_argument& ia)
                        {
                            expect_pdu_next = false;
                            current_message_idx = -1;
                        }
                        catch (const std::out_of_range& oor)
                        {
                            expect_pdu_next = false;
                            current_message_idx = -1;
                        }
                    }
                    else
                    {
                        expect_pdu_next = false;
                        current_message_idx = -1;
                    }
                }
                else if (expect_pdu_next && current_message_idx != -1)
                {
                    std::string pdu_data_line = line;
                    processAndStoreSms(pdu_data_line, current_message_idx);
                    messages_found_and_processed++;
                    expect_pdu_next = false;
                    current_message_idx = -1;
                }
                else if (isEndIdentifier(line) || isURC(line))
                {
                    if (expect_pdu_next)
                    {
                        expect_pdu_next = false;
                        current_message_idx = -1;
                    }
                }
            }

            if (command_ok)
            {
                if (messages_found_and_processed > 0)
                {
                }
                else
                {
                }
            }
            else if (command_error)
            {
                reboot();
            }
            else if (messages_found_and_processed > 0)
            {
            }
            else
            {
            }
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    // --- Internal Update Functions (Network, Voltage, etc.) ---
    static void updateNetworkQualityInternal()
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CSQ";
        request->callback = [](const std::string& response) -> bool
        {
            int rssi = 99;
            int ber = 99;
            bool success = false;
            auto pos = response.find("+CSQ:");
            if (pos != std::string::npos)
            {
                std::string values = response.substr(pos + 5);
                std::stringstream ss(values);
                char comma;
                ss >> std::ws >> rssi >> std::ws >> comma >> std::ws >> ber;
                if (!ss.fail() && comma == ',')
                    success = true;
            }
            if (success)
            {
                currentRssi = rssi;
                currentBer = ber;
                networkQualityValid = true;
                lastQualityUpdateTime = std::chrono::steady_clock::now();
            }
            else
            {
                networkQualityValid = false;
            }
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    static void updateGprsAttachmentStatusInternal()
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CGATT?";
        request->callback = [](const std::string& response) -> bool
        {
            bool success = false;
            bool attached = false;
            auto pos = response.find("+CGATT:");
            if (pos != std::string::npos)
            {
                std::string value_str = response.substr(pos + 7);
                value_str.erase(0, value_str.find_first_not_of(" \t\r\n"));
                if (!value_str.empty())
                {
                    try
                    {
                        int status = std::stoi(value_str);
                        attached = (status == 1);
                        success = true;
                    }
                    catch (...)
                    {
                        success = false;
                    }
                }
                else
                {
                    success = false;
                }
            }
            else if (response.find("OK") != std::string::npos &&
                     response.find("ERROR") == std::string::npos)
            {
                attached = false;
                success = true;
            }
            else if (response.find("ERROR") != std::string::npos)
            {
                attached = false;
                success = true;
            }

            if (success)
            {
                isGprsAttached = attached;
                gprsStateValid = true;
                lastGprsUpdateTime = std::chrono::steady_clock::now();
            }
            else
            {
                gprsStateValid = false;
            }
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    static void updateFlightModeStatusInternal()
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CFUN?";
        request->callback = [](const std::string& response) -> bool
        {
            bool success = false;
            bool flightMode = false;
            auto pos = response.find("+CFUN:");
            if (pos != std::string::npos)
            {
                std::string value = response.substr(pos + 6);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                try
                {
                    int funLevel = std::stoi(value);
                    if (funLevel == 4 || funLevel == 0)
                        flightMode = true;
                    else
                        flightMode = false;
                    success = true;
                }
                catch (...)
                {
                }
            }
            if (success)
            {
                flightModeState = flightMode;
                flightModeStateValid = true;
                lastFlightModeUpdateTime = std::chrono::steady_clock::now();
            }
            else
            {
                flightModeStateValid = false;
            }
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    static void updateVoltageInternal()
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CBC";
        request->callback = [](const std::string& response) -> bool
        {
            std::stringstream ss_block(response);
            std::string line;
            bool found_cbc_line = false;
            bool parsed_value_ok = false;
            bool cmd_success = (response.find("OK") != std::string::npos);
            int voltage = -1;

            while (std::getline(ss_block, line, '\n'))
            {
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);

                if (line.rfind("+CBC:", 0) == 0)
                {
                    found_cbc_line = true;
                    std::string data_part = line.substr(5);
                    data_part.erase(0, data_part.find_first_not_of(" \t"));

                    size_t first_comma = data_part.find(',');
                    size_t second_comma = std::string::npos;
                    if (first_comma != std::string::npos)
                        second_comma = data_part.find(',', first_comma + 1);
                    std::string voltage_str_raw;
                    if (first_comma != std::string::npos && second_comma != std::string::npos)
                        voltage_str_raw = data_part.substr(second_comma + 1);
                    else
                        voltage_str_raw = data_part;
                    voltage_str_raw.erase(
                        std::remove_if(
                            voltage_str_raw.begin(),
                            voltage_str_raw.end(),
                            [](char c)
                            {
                                return !std::isdigit(c);
                            }
                        ),
                        voltage_str_raw.end()
                    );
                    try
                    {
                        if (!voltage_str_raw.empty())
                        {
                            voltage = std::stoi(voltage_str_raw);
                            parsed_value_ok = true;
                        }
                        else
                        {
                        }
                    }
                    catch (const std::invalid_argument& ia)
                    {
                    }
                    catch (const std::out_of_range& oor)
                    {
                    }
                    break;
                }
            }
            bool update_successful = false;
            if (found_cbc_line && parsed_value_ok)
            {
                currentVoltage_mV = voltage;
                try
                {
                    battery_voltage_history.push_back(currentVoltage_mV);
                    if (battery_voltage_history.size() > 24)
                        battery_voltage_history.erase(battery_voltage_history.begin());
                    if (battery_voltage_history.size() > 0)
                    {
                        double sum = 0;
                        for (auto v : battery_voltage_history) sum += v;
                        currentVoltage_mV = sum / battery_voltage_history.size();
                    }
                }
                catch (const std::exception& e)
                {
                }

                voltageValid = true;
                lastVoltageUpdateTime = std::chrono::steady_clock::now();
                update_successful = true;
            }

            if (!update_successful)
                voltageValid = false;
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    double getBatteryLevel()
    {
#ifdef ESP_PLATFORM
        if (currentVoltage_mV == -1)
            return 1;
        const double voltage_V = currentVoltage_mV / 1000.0;
        const double batteryLevel = 3.083368 * std::pow(voltage_V, 3) -
                                    37.21203 * std::pow(voltage_V, 2) + 150.5735 * voltage_V -
                                    203.3347;
        return std::clamp(batteryLevel, 0.0, 1.0);
#else
        return 1;
#endif
    }

    static void updatePinStatusInternal()
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CPIN?";
        request->callback = [](const std::string& response) -> bool
        {
            std::stringstream ss_block(response);
            std::string line;
            std::string parsed_status = "";
            bool found_cpin_line = false;
            bool cmd_success = (response.find("OK") != std::string::npos);

            while (std::getline(ss_block, line, '\n'))
            {
                if (!line.empty() && line.back() == '\r')
                    line.pop_back();
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);
                if (line.rfind("+CPIN:", 0) == 0)
                {
                    parsed_status = line.substr(6);
                    parsed_status.erase(0, parsed_status.find_first_not_of(" \t"));
                    parsed_status.erase(parsed_status.find_last_not_of(" \t") + 1);
                    found_cpin_line = true;
                    break;
                }
            }
            bool needsPin = true;
            bool status_determined = false;

            if (found_cpin_line)
            {
                if (parsed_status == "READY")
                {
                    needsPin = false;
                    status_determined = true;
                }
                else if (parsed_status == "SIM PIN" || parsed_status == "SIM PUK")
                {
                    needsPin = true;
                    status_determined = true;
                }
                else if (parsed_status == "NOT INSERTED" ||
                         parsed_status.find("NO SIM") != std::string::npos ||
                         parsed_status.find("NOT READY") != std::string::npos)
                {
                    needsPin = true;
                    status_determined = true;
                }
                else
                {
                    needsPin = true;
                    status_determined = true;
                }
            }
            else if (!cmd_success && response.find("ERROR") != std::string::npos)
            {
                needsPin = true;
                status_determined = false;
            }
            else if (cmd_success && !found_cpin_line)
            {
                needsPin = true;
                status_determined = false;
            }
            else
            {
                needsPin = true;
                status_determined = false;
            }

            if (status_determined)
            {
                pinRequiresPin = needsPin;
                pinStatusValid = true;
                lastPinStatusUpdateTime = std::chrono::steady_clock::now();
            }
            else
            {
                pinStatusValid = false;
            }
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    static void updatePduModeStatusInternal()
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CMGF?";
        request->callback = [](const std::string& response) -> bool
        {
            bool success = false;
            bool pduMode = false;
            auto pos = response.find("+CMGF:");
            if (pos != std::string::npos)
            {
                std::string value = response.substr(pos + 6);
                value.erase(0, value.find_first_not_of(" \t\r\n"));
                value.erase(value.find_last_not_of(" \t\r\n") + 1);
                try
                {
                    int mode = std::stoi(value);
                    if (mode == 0)
                        pduMode = true;
                    else
                        pduMode = false;
                    success = true;
                }
                catch (...)
                {
                }
            }
            if (success)
            {
                pduModeEnabled = pduMode;
                pduModeStateValid = true;
                lastPduModeUpdateTime = std::chrono::steady_clock::now();
            }
            else
            {
                pduModeStateValid = false;
            }
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    // --- Initialization ---
    void init()
    {
#ifdef ESP_PLATFORM
        pinMode(GSM_PWR_PIN, OUTPUT);
        digitalWrite(GSM_PWR_PIN, 0);
        gsm.setRxBufferSize(4096);

        digitalWrite(GSM_PWR_PIN, 1);
        delay(1500);
        digitalWrite(GSM_PWR_PIN, 0);
        delay(3000);

        digitalWrite(GSM_PWR_PIN, 1);
        delay(1200);
        digitalWrite(GSM_PWR_PIN, 0);
        delay(10000);

        bool comm_ok = false;
        int attempts = 0;
        const int max_attempts = 3;

        gsm.begin(921600, SERIAL_8N1, RX, TX);
        delay(100);

        while (attempts < max_attempts && !comm_ok)
        {
            gsm.println("AT");
            delay(500);
            String data = "";
            while (gsm.available()) data += (char) gsm.read();
            Serial.print("[GSM RX @921600]: ");
            Serial.println(data);

            if (data.indexOf("OK") != -1)
                comm_ok = true;
            else
                delay(2000);
            attempts++;
        }

        if (!comm_ok)
        {
            gsm.updateBaudRate(115200);
            delay(100);
            attempts = 0;
            while (attempts < max_attempts && !comm_ok)
            {
                gsm.println("AT");
                delay(500);
                String data = "";
                while (gsm.available()) data += (char) gsm.read();
                Serial.print("[GSM RX @115200]: ");
                Serial.println(data);

                if (data.indexOf("OK") != -1)
                {
                    gsm.println("AT+IPR=921600");
                    delay(100);
                    gsm.flush();
                    gsm.updateBaudRate(921600);
                    delay(100);
                    gsm.println("AT");
                    delay(500);
                    data = "";
                    while (gsm.available()) data += (char) gsm.read();
                    Serial.print("[GSM RX @921600]: ");
                    Serial.println(data);
                    if (data.indexOf("OK") != -1)
                    {
                        comm_ok = true;
                    }
                    else
                    {
                        gsm.updateBaudRate(115200);
                        delay(100);
                        comm_ok = true;
                    }
                }
                else
                {
                    delay(2000);
                }
                attempts++;
            }
        }

        if (comm_ok)
        {
            currentCallState = CallState::IDLE;

            updatePinStatusInternal();
            updateGprsAttachmentStatusInternal();
            updateFlightModeStatusInternal();
            updatePduModeStatusInternal();
            updateNetworkQualityInternal();
            updateVoltageInternal();
            Time::syncNetworkTime();

            delay(500);
            uploadSettings();
            return;
        }

        currentCallState = CallState::UNKNOWN;
#else
        currentCallState = CallState::IDLE;
#endif
    }

    void reboot()
    {
        auto req = std::make_shared<Request>();
        req->command = "AT+CFUN=1,1";
        req->callback = [](const std::string& response) -> bool
        {
            init();
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(req);
    }

    void uploadSettings()
    {
        auto requestClip = std::make_shared<Request>();
        requestClip->command = "AT+CLIP=1";
        requestClip->callback = [](const std::string& response) -> bool
        {
            return false;
        };
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            requests.push_back(requestClip);
        }

        auto requestClcc = std::make_shared<Request>();
        requestClcc->command = "AT+CLCC=1";
        requestClcc->callback = [](const std::string& response) -> bool
        {
            return false;
        };
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            requests.push_back(requestClcc);
        }

        auto requestCnmi = std::make_shared<Request>();
        requestCnmi->command = "AT+CNMI=2,1,0,0,0";
        requestCnmi->callback = [](const std::string& response) -> bool
        {
            return false;
        };
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            requests.push_back(requestCnmi);
        }

        auto requestHourSync = std::make_shared<Request>();
        requestHourSync->command = "AT+CNTP=\"time.google.com\",8";
        requestHourSync->callback = [](const std::string& response) -> bool
        {
            return false;
        };
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            requests.push_back(requestHourSync);
        }

        auto requestMinuteSync = std::make_shared<Request>();
        requestMinuteSync->command = "AT+CNTP";
        requestMinuteSync->callback = [](const std::string& response) -> bool
        {
            return false;
        };
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            requests.push_back(requestMinuteSync);
        }

        auto requestCreg = std::make_shared<Request>();
        requestCreg->command = "AT+CREG=1";
        requestCreg->callback = [](const std::string& response) -> bool
        {
            return false;
        };
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            requests.push_back(requestCreg);
        }

        auto requestCgreg = std::make_shared<Request>();
        requestCgreg->command = "AT+CGREG=1";
        requestCgreg->callback = [](const std::string& response) -> bool
        {
            if (response.find("OK") != std::string::npos)
                updateGprsAttachmentStatusInternal();
            return false;
        };
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            requests.push_back(requestCgreg);
        }

        setPduMode(
            true,
            [](bool success)
            {
                if (success)
                {
                    checkForMessages();
                }
                else
                {
                }
            }
        );
    }

    // --- Corrected function ---
    bool isEndIdentifier(const std::string& data)
    {
        if (data == "OK" || data == "ERROR" || data == "NO CARRIER" || data == "BUSY" ||
            data == "NO ANSWER" || data == "NO DIALTONE" || data == "DOWNLOAD")
        { // <-- ADD THIS CHECK
            return true;
        }
        if (data.rfind("+CME ERROR:", 0) == 0 || data.rfind("+CMS ERROR:", 0) == 0)
            return true;
        return false;
    }

    const std::vector<std::string> known_urc_prefixes = {
        "RING",
        "+CLIP:",
        "+CLCC:",
        "NO CARRIER",
        "BUSY",
        "NO ANSWER",
        "+CMTI:",
        "+CMT:",
        "+CBM:",
        "+CDS:",
        "+CREG:",
        "+CGREG:",
        "+CEREG:",
        "+PDP:",
        "+SAPBR:",
        "Call Ready",
        "SMS Ready",
        "NORMAL POWER DOWN",
        "UNDER-VOLTAGE WARNING",
        "UNDER-VOLTAGE POWER DOWN",
        "+SIMREADY",
        "+RECEIVE",
        "CLOSED",
        "REMOTE IP:",
        "+IP_INITIAL",
        "+IP_STATUS",
        "+PDP_DEACT",
        "+SAPBR_DEACT",
        "+LOCATION:",
        "+CIPGSMLOC:",
        "+HTTPACTION:",
        "+FTPGET:",
        "+CGEV:"
    };

    bool isURC(const std::string& data)
    {
        if (data.empty())
            return false;
        for (const std::string& prefix : known_urc_prefixes)
            if (data.find(prefix) == 0)
                return true;
        return false;
    }

    void processURC(std::string data)
    {
        if (data.find("RING") == 0)
        {
            currentCallState = CallState::RINGING;
            lastIncomingCallNumber = "";
        }
        else if (data.find("+CLIP:") == 0)
        {
            size_t firstQuote = data.find('"');
            size_t secondQuote = data.find('"', firstQuote + 1);
            if (firstQuote != std::string::npos && secondQuote != std::string::npos)
            {
                lastIncomingCallNumber = data.substr(firstQuote + 1, secondQuote - firstQuote - 1);
                if (ExternalEvents::onIncommingCall)
                    ExternalEvents::onIncommingCall();
                if (currentCallState != CallState::RINGING)
                    currentCallState = CallState::RINGING;
            }
        }
        else if (data.rfind("+HTTPACTION:", 0) == 0)
        {
            if (currentHttpState != HttpState::ACTION_IN_PROGRESS)
                return;

            int method_urc, statusCode = 0, dataLen = 0;
            sscanf(data.c_str(), "+HTTPACTION: %d,%d,%d", &method_urc, &statusCode, &dataLen);
            std::cout << "[HTTP URC] Method: " << method_urc << ", Status: " << statusCode
                      << ", Data Length: " << dataLen << std::endl;

            if (currentHttpRequestDetails && currentHttpRequestDetails->on_response)
                currentHttpRequestDetails->on_response(statusCode);

            if (statusCode >= 200 && statusCode < 400)
            { // Handle success and redirects
                std::cout << "[HTTP URC] Request successful, data length: " << dataLen << std::endl;
                if (dataLen > 0)
                {
                    httpBytesTotal = dataLen;
                    httpBytesRead = 0;
                    currentHttpState = HttpState::READING;
                    std::cout << "[HTTP URC] Queueing next HTTP read." << std::endl;
                    _queueNextHttpRead();
                }
                else
                {
                    std::cout << "[HTTP URC] No data received, completing request." << std::endl;
                    _completeHttpRequest(HttpResult::OK);
                }
            }
            else if (statusCode >= 400)
            {
                std::cout << "[HTTP URC] Request failed with status code: " << statusCode
                          << std::endl;
                _completeHttpRequest(HttpResult::SERVER_ERROR);
            }
            else
            {
                std::cout << "[HTTP URC] Connection failed or no data received." << std::endl;
                _completeHttpRequest(HttpResult::CONNECTION_FAILED);
            }
        }
        else if (data.find("+CLCC:") == 0)
        {
            std::stringstream ss_clcc(data.substr(6));
            std::string clcc_part;
            std::vector<std::string> clcc_parts;
            while (std::getline(ss_clcc, clcc_part, ',')) clcc_parts.push_back(clcc_part);

            if (clcc_parts.size() >= 3)
            {
                try
                {
                    int stat = std::stoi(clcc_parts[2]);
                    CallState previousState = currentCallState;
                    switch (stat)
                    {
                    case 0:
                        currentCallState = CallState::ACTIVE;
                        break;
                    case 2:
                        currentCallState = CallState::DIALING;
                        break;
                    case 3:
                        currentCallState = CallState::DIALING;
                        break;
                    case 4:
                        currentCallState = CallState::RINGING;
                        break;
                    case 6:
                        currentCallState = CallState::IDLE;
                        break;
                    default:
                        break;
                    }
                    if (currentCallState != previousState)
                    {
                        if (currentCallState == CallState::IDLE)
                            lastIncomingCallNumber = "";
                    }
                }
                catch (...)
                {
                }
            }
            if (data == "+CLCC:")
            {
                if (currentCallState != CallState::IDLE)
                {
                    currentCallState = CallState::IDLE;
                    lastIncomingCallNumber = "";
                }
            }
        }
        else if (data == "NO CARRIER" || data == "BUSY" || data == "NO ANSWER")
        {
            if (currentCallState != CallState::IDLE)
            {
                currentCallState = CallState::IDLE;
                lastIncomingCallNumber = "";
            }
        }
        else if (data.find("+CMTI:") == 0)
        {
            size_t first_quote = data.find('"');
            size_t second_quote = data.find('"', first_quote + 1);
            size_t comma_after_quote = data.find(',', second_quote + 1);

            if (first_quote != std::string::npos && second_quote != std::string::npos &&
                comma_after_quote != std::string::npos)
            {
                std::string mem_store =
                    data.substr(first_quote + 1, second_quote - first_quote - 1);
                std::string index_str = data.substr(comma_after_quote + 1);
                index_str.erase(0, index_str.find_first_not_of(" \t"));
                index_str.erase(index_str.find_last_not_of(" \t") + 1);
                try
                {
                    int msg_idx = std::stoi(index_str);
                    queueReadSms(mem_store, msg_idx);
                }
                catch (const std::exception& e)
                {
                }
            }
            else
            {
            }
        }
        else if (data.find("+CREG:") == 0 || data.find("+CGREG:") == 0)
        {
            size_t comma_pos = data.find(',');
            if (comma_pos != std::string::npos)
            {
                try
                {
                    std::string stat_str = data.substr(comma_pos + 1);
                    size_t next_comma = stat_str.find(',');
                    if (next_comma != std::string::npos)
                        stat_str = stat_str.substr(0, next_comma);
                    stat_str.erase(0, stat_str.find_first_not_of(" \t"));
                    int stat = std::stoi(stat_str);
                    bool attached = (stat == 1 || stat == 5);

                    if (data.find("+CGREG:") == 0)
                    {
                        if (!gprsStateValid || isGprsAttached != attached)
                        {
                            isGprsAttached = attached;
                            gprsStateValid = true;
                            lastGprsUpdateTime = std::chrono::steady_clock::now();
                        }
                    }
                }
                catch (...)
                {
                }
            }
        }
        else if (data == "Call Ready" || data == "SMS Ready" || data == "+SIMREADY")
        {
            if (currentCallState == CallState::UNKNOWN)
                currentCallState = CallState::IDLE;
            if (!pinStatusValid)
                updatePinStatusInternal();
        }
        else if (data.find("+CGEV:") == 0)
        {
            if (data.find("DETACH") != std::string::npos)
            {
                if (isGprsAttached)
                {
                    isGprsAttached = false;
                    gprsStateValid = true;
                    lastGprsUpdateTime = std::chrono::steady_clock::now();
                }
            }
        }
    }

    // --- Public Accessors (Getters) ---
    std::pair<int, int> getNetworkQuality()
    {
        return {currentRssi, currentBer};
    }

    bool isNetworkQualityValid()
    {
        return networkQualityValid;
    }

    bool isConnected()
    {
        return isGprsAttached;
    }

    bool isConnectedStateValid()
    {
        return gprsStateValid;
    }

    bool isFlightModeActive()
    {
        return flightModeState;
    }

    bool isFlightModeStateValid()
    {
        return flightModeStateValid;
    }

    int getVoltage()
    {
        return currentVoltage_mV;
    }

    bool isVoltageValid()
    {
        return voltageValid;
    }

    bool isPinRequired()
    {
        return pinRequiresPin;
    }

    bool isPinStatusValid()
    {
        return pinStatusValid;
    }

    bool isPduModeEnabled()
    {
        return pduModeEnabled;
    }

    bool isPduModeStateValid()
    {
        return pduModeStateValid;
    }

    CallState getCallState()
    {
        return currentCallState;
    }

    std::string getLastIncomingNumber()
    {
        return lastIncomingCallNumber;
    }

    // --- Public Action Functions ---
    void setPin(const std::string& pin, std::function<void(bool success)> completionCallback)
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CPIN=" + pin;
        request->callback = [completionCallback](const std::string& response) -> bool
        {
            bool success = (response.find("OK") != std::string::npos);
            if (success)
            {
                pinRequiresPin = false;
                pinStatusValid = true;
                lastPinStatusUpdateTime = std::chrono::steady_clock::now();
                updatePinStatusInternal();
            }
            if (completionCallback)
                completionCallback(success);
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    void setFlightMode(bool enableFlightMode, std::function<void(bool success)> completionCallback)
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CFUN=" + std::string(enableFlightMode ? "4" : "1");
        request->callback = [completionCallback,
                             enableFlightMode](const std::string& response) -> bool
        {
            bool success = (response.find("OK") != std::string::npos);
            if (success)
            {
                flightModeState = enableFlightMode;
                flightModeStateValid = true;
                lastFlightModeUpdateTime = std::chrono::steady_clock::now();
            }
            if (completionCallback)
                completionCallback(success);
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    void setPduMode(bool enablePdu, std::function<void(bool success)> completionCallback)
    {
        auto request = std::make_shared<Request>();
        request->command = "AT+CMGF=" + std::string(enablePdu ? "0" : "1");
        request->callback = [completionCallback, enablePdu](const std::string& response) -> bool
        {
            bool success = (response.find("OK") != std::string::npos);
            if (success)
            {
                pduModeEnabled = enablePdu;
                pduModeStateValid = true;
                lastPduModeUpdateTime = std::chrono::steady_clock::now();
            }
            if (completionCallback)
                completionCallback(success);
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    void sendMessagePDU(
        const std::string& pdu, int length,
        std::function<void(bool success, int messageRef)> completionCallback
    )
    {
        auto request1 = std::make_shared<Request>();
        request1->command = "AT+CMGS=" + std::to_string(length);
        auto request2 = std::make_shared<Request>();
        request2->command = pdu + "\x1A";

        request1->callback = [completionCallback](const std::string& response) -> bool
        {
            std::string trimmed_response = response;
            size_t last_char_pos = trimmed_response.find_last_not_of(" \t\r\n");
            if (last_char_pos != std::string::npos && trimmed_response[last_char_pos] == '>')
            {
                return true;
            }
            else
            {
                if (completionCallback)
                    completionCallback(false, -1);
                return false;
            }
        };

        request2->callback = [completionCallback](const std::string& response) -> bool
        {
            int messageRef = -1;
            bool success = false;
            auto pos = response.find("+CMGS:");
            if (pos != std::string::npos)
            {
                std::string mr_str = response.substr(pos + 6);
                size_t first_digit = mr_str.find_first_of("0123456789");
                size_t last_digit = mr_str.find_first_not_of("0123456789", first_digit);
                if (first_digit != std::string::npos)
                {
                    mr_str = mr_str.substr(
                        first_digit,
                        last_digit == std::string::npos ? std::string::npos
                                                        : last_digit - first_digit
                    );
                    try
                    {
                        messageRef = std::stoi(mr_str);
                    }
                    catch (...)
                    { /* ignore */
                    }
                }
                if (response.find("OK") != std::string::npos)
                    success = true;
            }
            else if (response.find("OK") != std::string::npos)
            {
                success = false;
            }
            else if (response.find("ERROR") != std::string::npos)
            {
                success = false;
            }
            if (completionCallback)
                completionCallback(success, messageRef);
            return false;
        };
        request1->next = request2;
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request1);
    }

    static std::string byteToHex(unsigned char byte)
    {
        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        return ss.str();
    }

    static std::string encodePhoneNumber(const std::string& number)
    {
        std::string encoded;
        int len = number.length();
        for (int i = 0; i < len; i += 2)
        {
            if (i + 1 < len)
            {
                encoded += number[i + 1];
                encoded += number[i];
            }
            else
            {
                encoded += 'F';
                encoded += number[i];
            }
        }
        return encoded;
    }

    static std::string pack7Bit(const std::string& message, int& septetCount)
    {
        std::string packedDataHex;
        std::vector<unsigned char> packedOctets;
        unsigned char currentOctet = 0;
        int bitsInCurrentOctet = 0;
        septetCount = 0;

        for (char c_char : message)
        {
            unsigned char septet;
            if (c_char == '@')
                septet = 0x00;
            else if (c_char == '\xA3')
                septet = 0x01;
            else if (c_char == '$')
                septet = 0x02;
            else if (c_char == '\xA5')
                septet = 0x03;
            else if (c_char == '\xE8')
                septet = 0x04;
            else if (c_char == '\xE9')
                septet = 0x05;
            else if (c_char == '\xF9')
                septet = 0x06;
            else if (c_char == '\xEC')
                septet = 0x07;
            else if (c_char == '\xF2')
                septet = 0x08;
            else if (c_char == '\xC7')
                septet = 0x09;
            else if (c_char == '\n')
                septet = 0x0A;
            else if (c_char == '\xD8')
                septet = 0x0B;
            else if (c_char == '\xF8')
                septet = 0x0C;
            else if (c_char == '\r')
                septet = 0x0D;
            else if (c_char == '\xC5')
                septet = 0x0E;
            else if (c_char == '\xE5')
                septet = 0x0F;
            else if (c_char == '_')
                septet = 0x11;
            else if (c_char == '\xC6')
                septet = 0x1C;
            else if (c_char == '\xE6')
                septet = 0x1D;
            else if (c_char == '\xDF')
                septet = 0x1E;
            else if (c_char == '\xC9')
                septet = 0x1F;
            else if (c_char == ' ')
                septet = 0x20;
            else if (c_char >= 'A' && c_char <= 'Z')
                septet = static_cast<unsigned char>(c_char);
            else if (c_char >= 'a' && c_char <= 'z')
                septet = static_cast<unsigned char>(c_char);
            else if (c_char >= '0' && c_char <= '9')
                septet = static_cast<unsigned char>(c_char);
            else
            {
                switch (c_char)
                {
                case '!':
                    septet = 0x21;
                    break;
                case '"':
                    septet = 0x22;
                    break;
                case '#':
                    septet = 0x23;
                    break;
                case '%':
                    septet = 0x25;
                    break;
                case '&':
                    septet = 0x26;
                    break;
                case '\'':
                    septet = 0x27;
                    break;
                case '(':
                    septet = 0x28;
                    break;
                case ')':
                    septet = 0x29;
                    break;
                case '*':
                    septet = 0x2A;
                    break;
                case '+':
                    septet = 0x2B;
                    break;
                case ',':
                    septet = 0x2C;
                    break;
                case '-':
                    septet = 0x2D;
                    break;
                case '.':
                    septet = 0x2E;
                    break;
                case '/':
                    septet = 0x2F;
                    break;
                case ':':
                    septet = 0x3A;
                    break;
                case ';':
                    septet = 0x3B;
                    break;
                case '<':
                    septet = 0x3C;
                    break;
                case '=':
                    septet = 0x3D;
                    break;
                case '>':
                    septet = 0x3E;
                    break;
                case '?':
                    septet = 0x3F;
                    break;
                default:
                    septet = 0x3F;
                    break;
                }
            }
            septet &= 0x7F;
            septetCount++;

            currentOctet |= (septet << bitsInCurrentOctet);
            int bits_from_septet_for_this_octet = std::min(7, 8 - bitsInCurrentOctet);

            bitsInCurrentOctet += 7;

            if (bitsInCurrentOctet >= 8)
            {
                packedOctets.push_back(currentOctet);
                bitsInCurrentOctet -= 8;
                currentOctet = (septet >> (7 - bitsInCurrentOctet));
            }
        }

        if (bitsInCurrentOctet > 0 && septetCount > 0)
            packedOctets.push_back(currentOctet);

        for (unsigned char oct : packedOctets) packedDataHex += byteToHex(oct);
        return packedDataHex;
    }

    std::pair<std::string, int>
        encodePduSubmit(const std::string& recipientNumber, const std::string& message)
    {
        std::string pdu = "";
        int cmgsLength = 0;
        pdu += "00";
        pdu += "01";
        cmgsLength++;
        pdu += "00";
        cmgsLength++;

        std::string cleanNumber = recipientNumber;
        bool isInternational = false;
        if (!cleanNumber.empty() && cleanNumber[0] == '+')
        {
            isInternational = true;
            cleanNumber = cleanNumber.substr(1);
        }
        cleanNumber.erase(
            std::remove_if(
                cleanNumber.begin(),
                cleanNumber.end(),
                [](char c)
                {
                    return !std::isdigit(c);
                }
            ),
            cleanNumber.end()
        );

        if (cleanNumber.empty())
            return {"", -1};
        pdu += byteToHex(static_cast<unsigned char>(cleanNumber.length()));
        cmgsLength++;
        pdu += (isInternational ? "91" : "81");
        cmgsLength++;
        std::string encodedNumber = encodePhoneNumber(cleanNumber);
        pdu += encodedNumber;
        cmgsLength += encodedNumber.length() / 2;
        pdu += "00";
        cmgsLength++;
        pdu += "00";
        cmgsLength++;

        int septetCount = 0;
        std::string packedUserData = pack7Bit(message, septetCount);
        if (septetCount > 160)
        {
        }
        pdu += byteToHex(static_cast<unsigned char>(septetCount));
        cmgsLength++;
        pdu += packedUserData;
        cmgsLength += static_cast<int>(std::ceil(septetCount * 7.0 / 8.0));

        std::transform(pdu.begin(), pdu.end(), pdu.begin(), ::toupper);
        return {pdu, cmgsLength};
    }

    void sendMySms(const std::string& recipient, const std::string& text)
    {
        std::pair<std::string, int> pduData = Gsm::encodePduSubmit(recipient, text);
        if (pduData.second == -1)
            return;
        std::string pduString = pduData.first;
        int cmgsLength = pduData.second;

        Gsm::sendMessagePDU(
            pduString,
            cmgsLength,
            [recipient, text](bool success, int messageRef)
            {
                if (success)
                {
                    Conversations::Conversation conv;
                    storage::Path convPath(
                        std::string(MESSAGES_LOCATION) + "/" + recipient + ".json"
                    );
                    if (convPath.exists())
                        Conversations::loadConversation(convPath, conv);
                    else
                        conv.number = recipient;
                    conv.messages.push_back({text, false, std::to_string(getCurrentTimestamp())});
                    Conversations::saveConversation(convPath, conv);
                }
                else
                {
                }
            }
        );
    }

    void call(const std::string& number, std::function<void(bool success)> completionCallback)
    {
        if (currentCallState != CallState::IDLE)
        {
            if (completionCallback)
                completionCallback(false);
            return;
        }
        auto request = std::make_shared<Request>();
        request->command = "ATD" + number + ";";
        request->callback = [completionCallback, number](const std::string& response) -> bool
        {
            bool success = (response.find("OK") != std::string::npos);
            if (success)
                currentCallState = CallState::DIALING;
            if (completionCallback)
                completionCallback(success);
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    void acceptCall(std::function<void(bool success)> completionCallback)
    {
        if (currentCallState != CallState::RINGING)
        {
            if (completionCallback)
                completionCallback(false);
            return;
        }
        auto request = std::make_shared<Request>();
        request->command = "ATA";
        request->callback = [completionCallback](const std::string& response) -> bool
        {
            bool success = (response.find("OK") != std::string::npos);
            if (success)
                currentCallState = CallState::ACTIVE;
            if (completionCallback)
                completionCallback(success);
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    void rejectCall(std::function<void(bool success)> completionCallback)
    {
        if (currentCallState == CallState::IDLE || currentCallState == CallState::UNKNOWN)
        {
            if (completionCallback)
                completionCallback(false);
            return;
        }
        auto request = std::make_shared<Request>();
        request->command = "AT+CHUP";
        request->callback = [completionCallback](const std::string& response) -> bool
        {
            bool success = (response.find("OK") != std::string::npos);
            if (success)
            {
                currentCallState = CallState::IDLE;
                lastIncomingCallNumber = "";
            }
            if (completionCallback)
                completionCallback(success);
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(request);
    }

    void httpRequest(HttpRequest request)
    {
        std::cout << "[HTTP] Starting httpRequest for URL: " << request.url << std::endl;
        if (currentHttpState != HttpState::IDLE)
        {
            std::cout << "[HTTP] Request busy, cannot start new request." << std::endl;
            if (request.on_complete)
                request.on_complete(HttpResult::BUSY);
            std::cerr << "HTTP request already in progress, cannot start a new one." << std::endl;
            return;
        }

        currentHttpState = HttpState::INITIALIZING;
        currentHttpRequestDetails = std::make_unique<HttpRequest>(std::move(request));
        httpBytesTotal = 0;
        httpBytesRead = 0;

        std::shared_ptr<Request> firstReq = nullptr;
        std::shared_ptr<Request> lastReq = nullptr;

        // --- HTTP Service Initialization ---
        auto initReq = std::make_shared<Request>();
        initReq->command = "AT+HTTPINIT";
        initReq->callback = [](const std::string& response) -> bool
        {
            std::cout << "[HTTP] AT+HTTPINIT response: " << response << std::endl;
            if (response.find("OK") == std::string::npos)
            {
                std::cout << "[HTTP] HTTPINIT failed." << std::endl;
                _completeHttpRequest(HttpResult::INIT_FAILED);
                return false;
            }
            return true;
        };
        firstReq = initReq;
        lastReq = initReq;

        // --- HTTPS Configuration (if applicable) ---
        if (currentHttpRequestDetails->url.rfind("https://", 0) == 0)
        {
            std::cout << "[HTTP] HTTPS URL detected, configuring SSL context." << std::endl;
            auto sslCfgReq = std::make_shared<Request>();
            // Associate this HTTP session with SSL context 0.
            sslCfgReq->command = "AT+HTTPPARA=\"SSLCFG\",0";
            sslCfgReq->callback = [](const std::string& response) -> bool
            {
                std::cout << "[HTTP] AT+HTTPPARA SSLCFG response: " << response << std::endl;
                if (response.find("OK") == std::string::npos)
                {
                    std::cout << "[HTTP] SSLCFG set failed." << std::endl;
                    _completeHttpRequest(HttpResult::MODULE_ERROR);
                    return false;
                }
                return true;
            };
            lastReq->next = sslCfgReq;
            lastReq = sslCfgReq;
        }

        // --- Check and activate PDP context ---
        auto checkPdpReq = std::make_shared<Request>();
        checkPdpReq->command = "AT+CGACT?";
        checkPdpReq->callback = [](const std::string& response) -> bool
        {
            std::cout << "[HTTP] AT+CGACT? response: " << response << std::endl;
            if (response.find("+CGACT: 1,1") == std::string::npos)
                std::cout << "[HTTP] PDP context 1 not active, need to activate." << std::endl;
            return true;
        };
        lastReq->next = checkPdpReq;
        lastReq = checkPdpReq;

        auto activatePdpReq = std::make_shared<Request>();
        activatePdpReq->command = "AT+CGACT=1,1";
        activatePdpReq->callback = [](const std::string& response) -> bool
        {
            std::cout << "[HTTP] AT+CGACT=1,1 response: " << response << std::endl;
            if (response.find("OK") == std::string::npos &&
                response.find("ERROR") != std::string::npos)
            {
                std::cout << "[HTTP] PDP context activation failed." << std::endl;
                _completeHttpRequest(HttpResult::MODULE_ERROR);
                return false;
            }
            return true;
        };
        lastReq->next = activatePdpReq;
        lastReq = activatePdpReq;

        // --- HTTP Parameter Configuration ---
        // Set URL
        auto setUrlReq = std::make_shared<Request>();
        setUrlReq->command = "AT+HTTPPARA=\"URL\",\"" + currentHttpRequestDetails->url + "\"";
        setUrlReq->callback = [](const std::string& response) -> bool
        {
            std::cout << "[HTTP] AT+HTTPPARA URL response: " << response << std::endl;
            if (response.find("OK") == std::string::npos)
            {
                std::cout << "[HTTP] URL set failed." << std::endl;
                _completeHttpRequest(HttpResult::MODULE_ERROR);
                return false;
            }
            return true;
        };
        lastReq->next = setUrlReq;
        lastReq = setUrlReq;

        // Set Content-Type header if present
        for (const auto& header : currentHttpRequestDetails->headers)
        {
            if (header.first == "Content-Type")
            {
                std::cout << "[HTTP] Setting Content-Type: " << header.second << std::endl;
                auto headerReq = std::make_shared<Request>();
                headerReq->command = "AT+HTTPPARA=\"CONTENT\",\"" + header.second + "\"";
                headerReq->callback = [](const std::string& response) -> bool
                {
                    std::cout << "[HTTP] AT+HTTPPARA CONTENT response: " << response << std::endl;
                    if (response.find("OK") == std::string::npos)
                    {
                        std::cout << "[HTTP] Content-Type set failed." << std::endl;
                        _completeHttpRequest(HttpResult::MODULE_ERROR);
                        return false;
                    }
                    return true;
                };
                lastReq->next = headerReq;
                lastReq = headerReq;
            }
        }

        // POST-specific part
        if (currentHttpRequestDetails->method == HttpMethod::POST &&
            !currentHttpRequestDetails->body.empty())
        {
            std::cout << "[HTTP] POST method detected, preparing body." << std::endl;
            auto setDataReq = std::make_shared<Request>();
            setDataReq->command =
                "AT+HTTPDATA=" + std::to_string(currentHttpRequestDetails->body.length()) +
                ",10000";
            setDataReq->callback = [](const std::string& response) -> bool
            {
                std::cout << "[HTTP] AT+HTTPDATA response: " << response << std::endl;
                if (response.find("DOWNLOAD") != std::string::npos)
                    return true;
                std::cout << "[HTTP] HTTPDATA failed." << std::endl;
                _completeHttpRequest(HttpResult::MODULE_ERROR);
                return false;
            };
            lastReq->next = setDataReq;
            lastReq = setDataReq;

            auto sendBodyReq = std::make_shared<Request>();
            sendBodyReq->command = currentHttpRequestDetails->body;
            sendBodyReq->callback = [](const std::string& response) -> bool
            {
                std::cout << "[HTTP] Body send response: " << response << std::endl;
                if (response.find("OK") != std::string::npos)
                    return true;
                std::cout << "[HTTP] Body send failed." << std::endl;
                _completeHttpRequest(HttpResult::MODULE_ERROR);
                return false;
            };
            lastReq->next = sendBodyReq;
            lastReq = sendBodyReq;
        }

        // Action (GET or POST)
        auto actionReq = std::make_shared<Request>();
        actionReq->command = (currentHttpRequestDetails->method == HttpMethod::POST)
                                 ? "AT+HTTPACTION=1"
                                 : "AT+HTTPACTION=0";
        actionReq->callback = [](const std::string& response) -> bool
        {
            std::cout << "[HTTP] AT+HTTPACTION response: " << response << std::endl;
            if (response.find("OK") == std::string::npos)
            {
                std::cout << "[HTTP] HTTPACTION failed." << std::endl;
                _completeHttpRequest(HttpResult::MODULE_ERROR);
                return false;
            }
            std::cout << "[HTTP] HTTPACTION succeeded, waiting for URC." << std::endl;
            currentHttpState = HttpState::ACTION_IN_PROGRESS;
            return false;
        };
        lastReq->next = actionReq;

        // --- Queue the entire chain ---
        std::cout << "[HTTP] Queuing HTTP request chain." << std::endl;
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(firstReq);
    }

    static bool onHttpReadBlock(const std::string& response)
    {
        // Find the start of the data length header
        size_t header_start = response.find("+HTTPREAD:");
        if (header_start == std::string::npos)
        {
            // If the header isn't here, it might just be an "OK" for a zero-byte read.
            if (response.find("OK") != std::string::npos)
            {
                //_completeHttpRequest(HttpResult::OK);
            }
            else
            {
                std::cout << "[HTTP Read Error] Invalid response format." << std::endl;
                _completeHttpRequest(HttpResult::READ_ERROR);
            }
            return false;
        }

        return false; // This callback never chains to a 'next' request.
    }

    static void _queueNextHttpRead()
    {
        auto readReq = std::make_shared<Request>();
        readReq->command = "AT+HTTPREAD=1024";
        readReq->callback = onHttpReadBlock;
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.push_back(readReq);
    }

    static void _completeHttpRequest(HttpResult result)
    {
        if (currentHttpRequestDetails && currentHttpRequestDetails->on_complete)
            currentHttpRequestDetails->on_complete(result);

        currentHttpRequestDetails.reset();
        currentHttpState = HttpState::TERMINATING;

        auto termReq = std::make_shared<Request>();
        termReq->command = "AT+HTTPTERM";
        termReq->callback = [](const std::string&) -> bool
        {
            currentHttpState = HttpState::IDLE;
            return false;
        };
        std::lock_guard<std::mutex> lock(requestMutex);
        requests.insert(requests.begin(), termReq);
    }

    // --- Public Refresh Functions ---
    void refreshNetworkQuality()
    {
        updateNetworkQualityInternal();
    }

    void refreshConnectionStatus()
    {
        updateGprsAttachmentStatusInternal();
    }

    void refreshFlightModeStatus()
    {
        updateFlightModeStatusInternal();
    }

    void refreshVoltage()
    {
        updateVoltageInternal();
    }

    void refreshPinStatus()
    {
        updatePinStatusInternal();
    }

    void refreshPduModeStatus()
    {
        updatePduModeStatusInternal();
    }

    // --- Main Processing Loop (run) ---
    void run()
    {
#ifdef ESP_PLATFORM
        enum class SerialRunState
        {
            NO_COMMAND,
            COMMAND_RUNNING,
            SENDING_PDU_DATA
        };
        static SerialRunState state = SerialRunState::NO_COMMAND;
        static std::chrono::steady_clock::time_point lastCommandTime;
        const std::chrono::milliseconds commandTimeoutDuration(15000);
        const std::chrono::milliseconds pduTimeoutDuration(30000);

        static std::string currentResponseBlock = "";
        currentResponseBlock.reserve(2048);
        static std::string lineBuffer;
        lineBuffer.reserve(2048);

        if (state == SerialRunState::NO_COMMAND)
        {
            std::lock_guard<std::mutex> lock(requestMutex);
            if (!requests.empty())
            {
                currentRequest = requests.front();
                requests.erase(requests.begin());

                currentResponseBlock = "";

                if (currentRequest->command.find('\x1A') != std::string::npos)
                {
                    gsm.print(currentRequest->command.c_str());
                    state = SerialRunState::SENDING_PDU_DATA;
                    lastCommandTime = std::chrono::steady_clock::now();
                }
                else
                {
                    gsm.print(currentRequest->command.c_str());
                    gsm.print('\r');
                    state = SerialRunState::COMMAND_RUNNING;
                    lastCommandTime = std::chrono::steady_clock::now();
                }
            }
        }

        static std::string incomingData;
        incomingData.clear();

#ifdef ESP_PLATFORM
        while (gsm.available())
        {
            char c = gsm.read();
            incomingData += c;
        }
#endif

        if (!incomingData.empty())
            lineBuffer += incomingData;

        size_t lineEndPos;
        while ((lineEndPos = lineBuffer.find('\n')) != std::string::npos)
        {
            std::string line = lineBuffer.substr(0, lineEndPos);
            lineBuffer.erase(0, lineEndPos + 1);

            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty())
                continue;

            if ((state == SerialRunState::COMMAND_RUNNING) && currentRequest &&
                line == currentRequest->command)
            {
                continue;
            }

            bool potentialURC = isURC(line);
            bool isFinalReply = isEndIdentifier(line);

            if (potentialURC && (state == SerialRunState::NO_COMMAND || !isFinalReply))
            {
                processURC(line);
                continue;
            }

            size_t datasize = 0;
            // Parse datasize if this is an HTTPREAD line
            if (line.rfind("+HTTPREAD:", 0) == 0)
            {
                int parsed_size = 0;
                if (sscanf(line.c_str(), "+HTTPREAD: %d", &parsed_size) == 1 && parsed_size > 0)
                    datasize = static_cast<size_t>(parsed_size);

                std::cout << "[HTTP] HTTPREAD datasize: " << datasize << std::endl;

                if (datasize > 0)
                {
                    std::string data;
                    data.resize(datasize);
                    int bytesRead = 0;

                    std::cout << "[DEBUG] Starting to read data, expected size: " << datasize
                              << std::endl;

                    // First, consume any data already in lineBuffer (after "+HTTPREAD:" line)
                    if (!lineBuffer.empty() && bytesRead < datasize)
                    {
                        size_t toCopy = std::min(lineBuffer.size(), datasize - bytesRead);
                        std::copy_n(lineBuffer.begin(), toCopy, data.begin() + bytesRead);
                        bytesRead += toCopy;
                        lineBuffer.erase(0, toCopy);
                    }

                    // Then, read from serial until we have enough bytes or timeout occurs
                    auto start_time = std::chrono::steady_clock::now();
                    const auto timeout_duration = std::chrono::seconds(5);

                    while (bytesRead < datasize)
                    {
                        if (gsm.available())
                        {
                            char c = gsm.read();
                            data[bytesRead++] = c;
                        }

                        auto now = std::chrono::steady_clock::now();
                        if (std::chrono::duration_cast<std::chrono::seconds>(now - start_time) >
                            timeout_duration)
                        {
                            std::cout << "[ERROR] Timeout while reading data from serial"
                                      << std::endl;
                            break;
                        }
                    }

                    std::cout << "[DEBUG] Total bytes read: " << bytesRead << std::endl;

                    if (bytesRead == datasize)
                    {
                        if (currentHttpRequestDetails && currentHttpRequestDetails->on_data)
                        {
                            std::string_view data_chunk(data.data(), bytesRead);
                            std::cout << "[DEBUG] Data chunk received, invoking on_data callback"
                                      << std::endl;
                            currentHttpRequestDetails->on_data(data_chunk);
                            httpBytesRead += bytesRead;
                            std::cout << "[DEBUG] Total HTTP bytes read so far: " << httpBytesRead
                                      << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "[DEBUG] Incomplete data read, expected: " << datasize
                                  << ", got: " << bytesRead << std::endl;
                    }
                }
                else // No more data to read, call completion
                {
                    _completeHttpRequest(HttpResult::OK);
                }

                if (line.find("OK") != std::string::npos)
                {
                    std::cout << "[HTTP] HTTPREAD completed, total bytes read: " << httpBytesRead
                              << std::endl;

                    if (httpBytesRead >= httpBytesTotal)
                    {
                        //_completeHttpRequest(HttpResult::OK);
                    }
                    else
                    {
                        _queueNextHttpRead();
                    }
                }
                else if (line.find("ERROR") != std::string::npos)
                {
                    std::cout << "[HTTP] HTTPREAD error, total bytes read: " << httpBytesRead
                              << std::endl;
                    _completeHttpRequest(HttpResult::READ_ERROR);
                }
            }

            if (state == SerialRunState::COMMAND_RUNNING ||
                state == SerialRunState::SENDING_PDU_DATA)
            {
                currentResponseBlock += line + "\n";
                bool isPduPrompt = (line == ">" && state == SerialRunState::COMMAND_RUNNING);

                if (isFinalReply || isPduPrompt)
                {
                    bool executeNext = false;
                    if (currentRequest && currentRequest->callback)
                        executeNext = currentRequest->callback(currentResponseBlock);

                    if (isPduPrompt && executeNext && currentRequest && currentRequest->next)
                    {
                        std::lock_guard<std::mutex> lock(requestMutex);
                        requests.insert(requests.begin(), currentRequest->next);
                    }
                    else if (isFinalReply)
                    {
                        if (executeNext && currentRequest && currentRequest->next)
                        {
                            std::lock_guard<std::mutex> lock(requestMutex);
                            requests.insert(requests.begin(), currentRequest->next);
                        }
                    }

                    currentRequest = nullptr;
                    state = SerialRunState::NO_COMMAND;
                    currentResponseBlock = "";
                }
            }
            else if (!potentialURC)
            {
            }
        }

        if (state != SerialRunState::NO_COMMAND)
        {
            auto timeout = (state == SerialRunState::SENDING_PDU_DATA) ? pduTimeoutDuration
                                                                       : commandTimeoutDuration;
            if ((std::chrono::steady_clock::now() - lastCommandTime) > timeout)
            {
                if (currentRequest && currentRequest->callback)
                    currentRequest->callback("TIMEOUT_ERROR");
                currentRequest = nullptr;
                currentResponseBlock = "";
                lineBuffer = "";
                state = SerialRunState::NO_COMMAND;
            }
        }
#endif
    }

    void loop()
    {
        eventHandlerGsm.setInterval(
            [&]()
            {
                refreshNetworkQuality();
            },
            5000
        );

        eventHandlerGsm.setInterval(
            [&]()
            {
                refreshConnectionStatus();
                refreshVoltage();
                refreshPinStatus();
            },
            30000
        );

        eventHandlerGsm.setInterval(
            [&]()
            {
                Time::syncNetworkTime();
            },
            5000
        );

        while (true)
        {
            StandbyMode::buisy_io.lock();
            bool tasks_remain;
            do {
                run();
                eventHandlerGsm.update();
                PaxOS_Delay(1);

                // Safely check the condition for the loop
                {
                    std::lock_guard<std::mutex> lock(requestMutex);
                    tasks_remain = (!requests.empty() || currentRequest != nullptr);
                }
            } while (tasks_remain);

            StandbyMode::buisy_io.unlock();
            PaxOS_Delay(4);
        }
    }

} // namespace Gsm
