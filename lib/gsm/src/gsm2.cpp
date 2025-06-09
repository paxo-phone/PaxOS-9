#include "gsm2.hpp" // Assuming gsm2.hpp is the header file name

#include <chrono>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <sstream>
#include <algorithm>
#include <atomic> // For thread-safe flags if needed
#include <cstdio> // For sscanf
#include <iostream>
#include <cmath> // For ceil
#include <iomanip> // For std::setw, std::setfill
#include <stdexcept> // For exceptions in helpers (optional)

#include <delay.hpp> // Assumes PaxOS_Delay
#include <standby.hpp> // Assumes StandbyMode
#include <threads.hpp> // Assumes eventHandlerBack

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

#include <conversation.hpp>
#include <contacts.hpp>
#include <filestream.hpp>

// Dummy implementation
inline long long getCurrentTimestamp() {
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch())
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

    namespace ExternalEvents
    {
        std::function<void(void)> onIncommingCall;
        std::function<void(void)> onNewMessage;
        std::function<void(void)> onNewMessageError;
    }

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
        static void updateTimeInternal() {
            auto request = std::make_shared<Request>();
            request->command = "AT+CCLK?";
            request->callback = [](const std::string& response) -> bool {
                int l_yy, l_MM, l_dd, l_hh, l_mm, l_ss, l_zz;
                char sign;
                bool parsed_ok = false;

                size_t start_pos = response.find("+CCLK:");
                if (start_pos != std::string::npos) {
                    size_t quote_start = response.find('"', start_pos);
                    size_t quote_end = std::string::npos;
                    if (quote_start != std::string::npos) {
                        quote_end = response.find('"', quote_start + 1);
                    }

                    if (quote_start != std::string::npos && quote_end != std::string::npos) {
                        std::string time_str = response.substr(quote_start + 1, quote_end - quote_start - 1);
                        int fields_read = sscanf(time_str.c_str(), "%d/%d/%d,%d:%d:%d%c%d",
                                                 &l_yy, &l_MM, &l_dd, &l_hh, &l_mm, &l_ss, &sign, &l_zz);

                        if (fields_read == 8) { 
                            if (l_yy >= 0 && l_yy <= 99 && l_MM >= 1 && l_MM <= 12 && l_dd >= 1 && l_dd <= 31 &&
                                l_hh >= 0 && l_hh <= 23 && l_mm >= 0 && l_mm <= 59 && l_ss >= 0 && l_ss <= 59 &&
                                (sign == '+' || sign == '-'))
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
                                // printf("[GSM Time] Updated: %04d/%02d/%02d %02d:%02d:%02d (Offset: %c%02d quarters)\n",
                                //       year, month, day, hour, minute, second, sign, l_zz);
                            } else {
                                // printf("[GSM Time] Parsed fields out of range: %s\n", time_str.c_str());
                            }
                        } else {
                            // printf("[GSM Time] Failed to parse time string with sscanf (Fields read: %d): %s\n", fields_read, time_str.c_str());
                        }
                    } else {
                         // printf("[GSM Time] Could not find quotes in +CCLK response.\n");
                    }
                } else {
                     // printf("[GSM Time] +CCLK: not found in response.\n");
                }

                if (!parsed_ok) {
                    // printf("[GSM Time] Failed to update time.\n");
                }
                return false; 
            };
            requests.push_back(request); 
        }

        void syncNetworkTime() {
            // printf("[GSM Time] Queuing network time sync (AT+CCLK?).\n");
            updateTimeInternal();
        }

        int getYear() { return timeValid ? year : -1; }
        int getMonth() { return timeValid ? month : -1; }
        int getDay() { return timeValid ? day : -1; }
        int getHour() { return timeValid ? hour : -1; }
        int getMinute() { return timeValid ? minute : -1; }
        int getSecond() { return timeValid ? second : -1; }
        int getTimezoneOffsetQuarterHours() { return timeValid ? timezoneOffsetQuarterHours : 0; }
        bool isTimeValid() { return timeValid; }

    } // namespace Time


    // --- PDU Decoding Structures and Enums (Internal) ---
    enum class SmsType {
        TEXT_SMS,
        MMS_NOTIFICATION,
        STATUS_REPORT,
        UNKNOWN
    };

    struct DecodedPdu {
        SmsType type = SmsType::UNKNOWN;
        std::string senderNumber;
        std::string messageContent;
        std::string serviceCenterTimestamp; // Format: "YYMMDDHHMMSS+/-ZZ"
        std::string mmsUrl; // If MMS_NOTIFICATION and URL is found
        bool hasUserDataHeader = false;
    };

    // --- PDU Decoding Helper Functions (Internal) ---
    static unsigned char hexPairToByte(char c1, char c2) {
        unsigned char byte = 0;
        auto charToVal = [](char c) -> unsigned char {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            throw std::invalid_argument("Invalid hex char");
        };
        byte = (charToVal(c1) << 4) | charToVal(c2);
        return byte;
    }

    static std::vector<unsigned char> hexStringToBytes(const std::string& hex) {
        if (hex.length() % 2 != 0) {
            throw std::invalid_argument("Hex string must have an even length.");
        }
        std::vector<unsigned char> bytes;
        for (size_t i = 0; i < hex.length(); i += 2) {
            bytes.push_back(hexPairToByte(hex[i], hex[i+1]));
        }
        return bytes;
    }

    static std::string decodeSemiOctets(const std::string& data_hex, int num_digits_to_read) {
        std::string result = "";
        for (int i = 0; i < num_digits_to_read; ++i) {
            int byte_idx = i / 2;
            char digit_char;
            if (i % 2 == 0) {
                digit_char = data_hex[byte_idx * 2 + 1];
            } else {
                digit_char = data_hex[byte_idx * 2];
            }
            // Handle potential padding 'F' at the end of an odd number of digits
            if (digit_char == 'F' || digit_char == 'f') {
                 if (i == num_digits_to_read -1 && (num_digits_to_read % 2 != 0) ) { /* valid padding, ignore this 'F' */ continue; }
            }
            result += digit_char;
        }
        return result;
    }

    static std::string gsm7SeptetToUtf8(unsigned char septet) {
        septet &= 0x7F; // Ensure only 7 bits are considered
        switch (septet) {
            case 0x00: return "@"; case 0x01: return "\xC2\xA3"; /* £ */ case 0x02: return "$";
            case 0x03: return "\xC2\xA5"; /* ¥ */ case 0x04: return "\xC3\xA8"; /* è */ case 0x05: return "\xC3\xA9"; /* é */
            case 0x06: return "\xC3\xB9"; /* ù */ case 0x07: return "\xC3\xAC"; /* ì */ case 0x08: return "\xC3\xB2"; /* ò */
            case 0x09: return "\xC3\x87"; /* Ç */ case 0x0A: return "\n";   case 0x0B: return "\xC3\x98"; /* Ø */
            case 0x0C: return "\xC3\xB8"; /* ø */ case 0x0D: return "\r";   case 0x0E: return "\xC3\x85"; /* Å */
            case 0x0F: return "\xC3\xA5"; /* å */
            // GSM 03.38 default alphabet mapping
            case 0x10: return "\xCE\x94"; /* Δ */
            case 0x11: return "_";
            case 0x12: return "\xCE\xA6"; /* Φ */
            case 0x13: return "\xCE\x93"; /* Γ */
            case 0x14: return "\xCE\x9B"; /* Λ */
            case 0x15: return "\xCE\xA9"; /* Ω */
            case 0x16: return "\xCE\xA0"; /* Π */
            case 0x17: return "\xCE\xA8"; /* Ψ */
            case 0x18: return "\xCE\xA3"; /* Σ */
            case 0x19: return "\xCE\x98"; /* Θ */
            case 0x1A: return "\xCE\x9E"; /* Ξ */
            case 0x1B: return " ";    /* ESC - map to space or ignore? Mapping to space like the old code implied */
            case 0x1C: return "\xC3\x86"; /* Æ */
            case 0x1D: return "\xC3\xA6"; /* æ */
            case 0x1E: return "\xC3\x9F"; /* ß */
            case 0x1F: return "\xC3\x89"; /* É */
            case 0x20: return " ";
            // 0x21-0x7E are generally printable ASCII and map directly (single byte UTF-8)
            case 0x21: return "!"; case 0x22: return "\""; case 0x23: return "#";
            // Note: 0x24 ($) is already handled by case 0x02, but if encountered here, 0x24 is also correct.
            case 0x25: return "%"; case 0x26: return "&"; case 0x27: return "'";
            case 0x28: return "("; case 0x29: return ")"; case 0x2A: return "*";
            case 0x2B: return "+"; case 0x2C: return ","; case 0x2D: return "-";
            case 0x2E: return "."; case 0x2F: return "/";
            case 0x30: return "0"; case 0x31: return "1"; case 0x32: return "2"; case 0x33: return "3";
            case 0x34: return "4"; case 0x35: return "5"; case 0x36: return "6"; case 0x37: return "7";
            case 0x38: return "8"; case 0x39: return "9"; case 0x3A: return ":"; case 0x3B: return ";";
            case 0x3C: return "<"; case 0x3D: return "="; case 0x3E: return ">"; case 0x3F: return "?";
            case 0x40: return "@"; // Also 0x00
            case 0x41: return "A"; case 0x42: return "B"; case 0x43: return "C"; case 0x44: return "D";
            case 0x45: return "E"; case 0x46: return "F"; case 0x47: return "G"; case 0x48: return "H";
            case 0x49: return "I"; case 0x4A: return "J"; case 0x4B: return "K"; case 0x4C: return "L";
            case 0x4D: return "M"; case 0x4E: return "N"; case 0x4F: return "O"; case 0x50: return "P";
            case 0x51: return "Q"; case 0x52: return "R"; case 0x53: return "S"; case 0x54: return "T";
            case 0x55: return "U"; case 0x56: return "V"; case 0x57: return "W"; case 0x58: return "X";
            case 0x59: return "Y"; case 0x5A: return "Z";
            case 0x5B: return "\xC3\xA4"; /* ä */ case 0x5C: return "\xC3\xB6"; /* ö */ case 0x5D: return "\xC3\xBC"; /* ü */
            case 0x5E: return "\xC3\x84"; /* Ä */ case 0x5F: return "_"; // Also 0x11
            case 0x60: return "\xC3\x9F"; /* ẞ - capital sharp s, sometimes mapped to 0x60 or 0x1E? Use 0x1E mapping */
            case 0x61: return "a"; case 0x62: return "b"; case 0x63: return "c"; case 0x64: return "d";
            case 0x65: return "e"; case 0x66: return "f"; case 0x67: return "g"; case 0x68: return "h";
            case 0x69: return "i"; case 0x6A: return "j"; case 0x6B: return "k"; case 0x6C: return "l";
            case 0x6D: return "m"; case 0x6E: return "n"; case 0x6F: return "o"; case 0x70: return "p";
            case 0x71: return "q"; case 0x72: return "r"; case 0x73: return "s"; case 0x74: return "t";
            case 0x75: return "u"; case 0x76: return "v"; case 0x77: return "w"; case 0x78: return "x";
            case 0x79: return "y"; case 0x7A: return "z";
            case 0x7B: return "\xC3\xA4"; /* ä */ // Also 0x5B - use 0x5B mapping? Let's be consistent
            case 0x7C: return "\xC3\xB6"; /* ö */ // Also 0x5C - use 0x5C mapping?
            case 0x7D: return "\xC3\xBC"; /* ü */ // Also 0x5D - use 0x5D mapping?
            // Original: case 0x7E: return "\xC3\xA0"; /* à */
            case 0x7E: return "a"; // Map 'à' to 'a' as a workaround for display limitations
            case 0x7F: return "\xE2\x82\xAC"; /* € (commonly mapped to 0x7F without ESC) */
            default:
                // Unhandled or undefined septets -> Map to '?'
                return "?";
        }
    }

    static char gsm7SeptetToChar(unsigned char septet) {
        switch (septet) {
            case 0x00: return '@'; case 0x01: return '\xA3'; case 0x02: return '$';
            case 0x03: return '\xA5'; case 0x04: return '\xE8'; case 0x05: return '\xE9';
            case 0x06: return '\xF9'; case 0x07: return '\xEC'; case 0x08: return '\xF2';
            case 0x09: return '\xC7'; case 0x0A: return '\n';   case 0x0B: return '\xD8';
            case 0x0C: return '\xF8'; case 0x0D: return '\r';   case 0x0E: return '\xC5';
            case 0x0F: return '\xE5'; 
            // GSM 03.38 defines 0x10-0x1A (excluding 0x1B ESC) as Greek letters or symbols.
            // Representing them as single 'char' is problematic if they are not in
            // the execution character set as single-byte entities.
            // Mapping to '?' to avoid warnings/overflow and indicate unrepresentable char.
            case 0x10: return '?'; /* Was GREEK CAPITAL LETTER DELTA \u0394 */
            case 0x11: return '_';
            case 0x12: return '?'; /* Was GREEK CAPITAL LETTER PHI \u03A6 */
            case 0x13: return '?'; /* Was GREEK CAPITAL LETTER GAMMA \u0393 */
            case 0x14: return '?'; /* Was GREEK CAPITAL LETTER LAMBDA \u039B */
            case 0x15: return '?'; /* Was GREEK CAPITAL LETTER OMEGA \u03A9 */
            case 0x16: return '?'; /* Was GREEK CAPITAL LETTER PI \u03A0 */
            case 0x17: return '?'; /* Was GREEK CAPITAL LETTER PSI \u03A8 */
            case 0x18: return '?'; /* Was GREEK CAPITAL LETTER SIGMA \u03A3 */
            case 0x19: return '?'; /* Was GREEK CAPITAL LETTER THETA \u0398 */
            case 0x1A: return '?'; /* Was GREEK CAPITAL LETTER XI \u039E */
            case 0x1B: return ' ';    /* ESC - map to space or '?' for simplicity as it's for extensions */
            case 0x1C: return '\xC6'; // Æ
            case 0x1D: return '\xE6'; // æ
            case 0x1E: return '\xDF'; // ß (eszett)
            case 0x1F: return '\xC9'; // É
            case 0x20: return ' ';
            default:
                // For 0x21-0x7A, many are direct ASCII mappings.
                // Check if the septet corresponds to a printable ASCII character.
                if ( (septet >= '0' && septet <= '9') ||
                     (septet >= 'A' && septet <= 'Z') ||
                     (septet >= 'a' && septet <= 'z') ||
                     std::string(" !\"#%&'()*+,-./:;<=>?").find(static_cast<char>(septet)) != std::string::npos) {
                     // Note: 0x24 ($) is already handled by case 0x02 as per GSM spec.
                     // If septet is 0x24 here, it would become '$', which is fine.
                    return static_cast<char>(septet);
                }
                // Other characters in this range might not be standard ASCII or already handled (e.g. '$')
                return '?'; // Default for unhandled septets in this range or outside
        }
    }

    static std::string unpack7BitData(const std::vector<unsigned char>& packed_octets, int num_septets, int udh_septet_length = 0) {
        std::string text = "";
        if (num_septets <= 0 || packed_octets.empty()) return text;
    
        int current_octet_idx = 0;
        int bit_shift = 0; // How many bits from the *next* octet are needed for the current septet
    
        for (int i = 0; i < num_septets; ++i) {
            if (current_octet_idx >= packed_octets.size()) {
                // printf("[PDU Decode] unpack7BitData: Ran out of octets for %d septets. Index %d.\n", num_septets, packed_octets.size() * 8 / 7, i);
                break;
            }
            unsigned char current_septet;
            
            // Extract bits for the current septet
            // The first part comes from the current octet, shifted right by bit_shift
            current_septet = (packed_octets[current_octet_idx] >> bit_shift);
    
            // The second part (if needed) comes from the next octet
            if (bit_shift > 0) { // We need bits from the next octet if bit_shift > 0
                 if (current_octet_idx + 1 < packed_octets.size()) {
                     current_septet |= (packed_octets[current_octet_idx + 1] << (8 - bit_shift));
                 } else if (i < num_septets - 1 || ( (i == num_septets - 1) && (bit_shift + (8-bit_shift) < 7) && packed_octets.size() * 8 / 7 < num_septets) ) {
                      // This check is a bit complex, essentially seeing if we legitimately expect more octets based on total septets
                      // or if we ran out prematurely. For now, a simple check is if we needed bits from the next octet but it doesn't exist.
                      // If bit_shift is > 0, we need 8-bit_shift bits from the next octet. If next_octet is out of bounds, and these
                      // bits were necessary (i.e., bit_shift + (8 - bit_shift) < 7 is false, which is always true... wait, logic error)
                      // A simpler check: If bit_shift > 0 and we're at the last octet index, and we still need more septets...
                      // The total number of bits is num_septets * 7. The number of octets needed is ceil(num_septets * 7.0 / 8.0).
                      // If current_octet_idx is the last valid index, and bit_shift > 0, and we still expect more septets (i < num_septets - 1),
                      // then something is wrong with the PDU length or data.
                      if (current_octet_idx == packed_octets.size() -1) {
                          // This happens for the very last septet if it crosses an octet boundary.
                          // The remaining bits of the last septet are in the last octet. No next octet is expected.
                          // The logic `current_septet |= (packed_octets[current_octet_idx + 1] << (8 - bit_shift));` will correctly
                          // not add bits if current_octet_idx + 1 is out of bounds because packed_octets[...] would likely throw or access garbage.
                          // Let's rely on bounds check `if (current_octet_idx + 1 < packed_octets.size())`
                       }
                 }
            }
    
            current_septet &= 0x7F; // Mask to get the 7-bit value
    
            // Only decode and append character if it's not part of the UDH
            if (i >= udh_septet_length) {
                text += gsm7SeptetToUtf8(current_septet);
            }
    
            bit_shift += 7;
            if (bit_shift >= 8) {
                current_octet_idx++;
                bit_shift -= 8;
            }
        }
        return text;
    }

    static std::string ucs2BytesToUtf8(const std::vector<unsigned char>& ucs2_bytes, int offset) {
        std::string utf8_text;
        for (size_t i = offset; i < ucs2_bytes.size(); i += 2) {
            if (i + 1 >= ucs2_bytes.size()) {
                // printf("[PDU Decode] UCS2 decode ran out of bytes (expected pairs).\n");
                utf8_text += "?"; // Append placeholder if pair is incomplete
                break;
            }
    
            // UCS-2 is UTF-16BE (Big Endian)
            unsigned short ucs2_code_unit = (static_cast<unsigned short>(ucs2_bytes[i]) << 8) | static_cast<unsigned short>(ucs2_bytes[i+1]);
    
            if (ucs2_code_unit <= 0x7F) {
                // 1-byte UTF-8 (ASCII range)
                utf8_text += static_cast<char>(ucs2_code_unit);
            } else if (ucs2_code_unit <= 0x7FF) {
                // 2-byte UTF-8
                utf8_text += static_cast<char>(0xC0 | (ucs2_code_unit >> 6));
                utf8_text += static_cast<char>(0x80 | (ucs2_code_unit & 0x3F));
            } else if (ucs2_code_unit >= 0xD800 && ucs2_code_unit <= 0xDFFF) {
                 // Surrogate pairs - UCS-2 technically doesn't have surrogates, this range is reserved.
                 // Treat as error or placeholder in standard UCS-2 decoding.
                 // printf("[PDU Decode] Encountered reserved surrogate range in UCS-2 data: 0x%04X\n", ucs2_code_unit);
                 utf8_text += "?"; // Placeholder for invalid/unhandled UCS-2
            }
            else {
                // 3-byte UTF-8 (BMP characters outside ASCII/7FF)
                utf8_text += static_cast<char>(0xE0 | (ucs2_code_unit >> 12));
                utf8_text += static_cast<char>(0x80 | ((ucs2_code_unit >> 6) & 0x3F));
                utf8_text += static_cast<char>(0x80 | (ucs2_code_unit & 0x3F));
            }
        }
        return utf8_text;
    }

    static bool decodePduDeliver(const std::string& pdu_hex_string, DecodedPdu& result) {
        try {
            int current_pos = 0;
            // Service Center Address (SCA) - Length and Value
            int sca_len_octets = hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos+1]);
            current_pos += 2;
            if (sca_len_octets > 0) {
                 // Skip SCA Type of Address and Value
                 if (current_pos + sca_len_octets * 2 > pdu_hex_string.length()) {
                     // printf("[PDU Decode] PDU string too short for SCA data.\n");
                     return false;
                 }
                 current_pos += sca_len_octets * 2;
            }
    
    
            // TPDU Type Octet (TP-MTI, TP-UDHI, etc.)
            if (current_pos + 2 > pdu_hex_string.length()) {
                 // printf("[PDU Decode] PDU string too short for TPDU Type.\n");
                 return false;
            }
            unsigned char tpdu_type_octet = hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos+1]);
            current_pos += 2;
            int tp_mti = tpdu_type_octet & 0x03; // Bits 0, 1 (Message Type Indicator)
            result.hasUserDataHeader = (tpdu_type_octet & 0x40) != 0; // Bit 6 (User Data Header Indicator)
    
            if (tp_mti == 0x00) { // TP-DELIVER (Inbound SMS)
                result.type = SmsType::TEXT_SMS; // Assume TEXT_SMS, might change to MMS_NOTIFICATION later
    
                // Originating Address (OA) - Length and Value
                if (current_pos + 2 > pdu_hex_string.length()) {
                     // printf("[PDU Decode] PDU string too short for OA length.\n");
                     return false;
                }
                int oa_len_digits = hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos+1]);
                current_pos += 2;
                if (current_pos + 2 > pdu_hex_string.length()) { // Type of Address (OA)
                     // printf("[PDU Decode] PDU string too short for OA Type.\n");
                     return false;
                }
                current_pos += 2; // Skip OA Type of Address (assuming 1 octet)
                int oa_addr_len_octets = (oa_len_digits + 1) / 2;
                if (current_pos + oa_addr_len_octets * 2 > pdu_hex_string.length()) {
                    // printf("[PDU Decode] PDU string too short for OA data.\n");
                    return false;
                }
                std::string oa_hex = pdu_hex_string.substr(current_pos, oa_addr_len_octets * 2);
                result.senderNumber = decodeSemiOctets(oa_hex, oa_len_digits);
                current_pos += oa_addr_len_octets * 2;
    
                // TP-PID (Protocol Identifier)
                if (current_pos + 2 > pdu_hex_string.length()) {
                    // printf("[PDU Decode] PDU string too short for TP-PID.\n");
                    return false;
                }
                current_pos += 2; // Skip TP-PID (assuming 1 octet)
    
                // TP-DCS (Data Coding Scheme)
                if (current_pos + 2 > pdu_hex_string.length()) {
                    // printf("[PDU Decode] PDU string too short for TP-DCS.\n");
                    return false;
                }
                unsigned char tp_dcs = hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos+1]);
                current_pos += 2;
    
                bool is_7bit_encoding = false;
                bool is_8bit_encoding = false;
                bool is_ucs2_encoding = false;
                unsigned char dcs_group = (tp_dcs & 0xC0);
                unsigned char dcs_char_set = (tp_dcs & 0x0C) >> 2;
    
                if (dcs_group == 0x00) { // Group 0xx
                     if ((tp_dcs & 0x20) == 0x00) { // 00xx
                        if (dcs_char_set == 0x00) is_7bit_encoding = true; // GSM 7 default
                        else if (dcs_char_set == 0x01) is_8bit_encoding = true; // 8 bit data
                        else if (dcs_char_set == 0x02) is_ucs2_encoding = true; // UCS-2
                     } else { // 01xx - Compressed or message class
                          is_7bit_encoding = true; // Default unless compressed (which we ignore)
                     }
                } else if (dcs_group == 0x40) { // Group 10xx
                     is_8bit_encoding = true; // 8 bit data (auto-detection or message class)
                } else if (dcs_group == 0x80) { // Group 10xx - Reserved
                     // Treat as unknown, potentially default 7-bit? PDU spec says reserved.
                     // printf("[PDU Decode] Reserved DCS group 10xx (0x%02X).\n", tp_dcs);
                     return false; // Indicate failure for clarity
                } else if (dcs_group == 0xC0) { // Group 11xx - Message class
                     if ((tp_dcs & 0x20) == 0x00) { // 110x - Message class
                         if (dcs_char_set == 0x00) is_7bit_encoding = true; // Class 0/1/2/3, GSM 7
                         else if (dcs_char_set == 0x01) is_8bit_encoding = true; // Class 0/1/2/3, 8 bit
                         else if (dcs_char_set == 0x02) is_ucs2_encoding = true; // Class 0/1/2/3, UCS-2
                     } else { // 111x - Reserved
                         // printf("[PDU Decode] Reserved DCS group 111x (0x%02X).\n", tp_dcs);
                         return false; // Indicate failure
                     }
                } else { // Should not happen with 0xC0 mask
                     // printf("[PDU Decode] Unknown DCS byte structure: 0x%02X\n", tp_dcs);
                     return false;
                }
    
    
                // TP-SCTS (Service Centre Time Stamp)
                if (current_pos + 7 * 2 > pdu_hex_string.length()) {
                     // printf("[PDU Decode] PDU string too short for SCTS.\n");
                     return false;
                }
                std::string scts_hex = pdu_hex_string.substr(current_pos, 7 * 2);
                // The format is YYMMDDhhmmss+ZZ or -ZZ (semi-octets)
                result.serviceCenterTimestamp = decodeSemiOctets(scts_hex, 14); // 7 octets hold 14 semi-octets/digits
                current_pos += 7 * 2;
    
    
                // TP-UDL (User Data Length)
                 if (current_pos + 2 > pdu_hex_string.length()) {
                    // printf("[PDU Decode] PDU string too short for TP-UDL.\n");
                    return false;
                }
                unsigned char tp_udl = hexPairToByte(pdu_hex_string[current_pos], pdu_hex_string[current_pos+1]);
                current_pos += 2;
    
                // User Data (UD)
                int ud_expected_octet_len = 0;
                if (is_7bit_encoding) {
                    // UDL is number of septets. Octet length is calculated.
                     ud_expected_octet_len = static_cast<int>(std::ceil(tp_udl * 7.0 / 8.0));
                } else if (is_8bit_encoding || is_ucs2_encoding) {
                    // UDL is number of octets.
                    ud_expected_octet_len = tp_udl;
                } else {
                     // printf("[PDU Decode] User Data: Unknown encoding determined by DCS 0x%02X.\n", tp_dcs);
                     return false;
                }
    
                if (current_pos + ud_expected_octet_len * 2 > pdu_hex_string.length()) {
                    // printf("[PDU Decode] PDU string too short for User Data (Expected %d octets).\n", ud_expected_octet_len);
                    // Attempt to read what's left, but this is likely corrupt data
                    ud_expected_octet_len = (pdu_hex_string.length() - current_pos) / 2;
                    // printf("[PDU Decode] Using remaining %d octets for UD.\n", ud_expected_octet_len);
                    if (ud_expected_octet_len < 0) ud_expected_octet_len = 0; // Safety
                    if (ud_expected_octet_len == 0) {
                         // printf("[PDU Decode] No User Data bytes available.\n");
                         result.messageContent = "";
                         // Proceed to return true if all previous mandatory fields were parsed
                         return true; 
                    }
                }
                std::string ud_hex = pdu_hex_string.substr(current_pos, ud_expected_octet_len * 2);
                std::vector<unsigned char> ud_bytes = hexStringToBytes(ud_hex);
    
    
                int udh_octet_len = 0; // Length of the UDH *field* including length byte
                int udh_septet_len = 0; // Length of UDH *in septets* for 7-bit decoding offset
                if (result.hasUserDataHeader && !ud_bytes.empty()) {
                    udh_octet_len = ud_bytes[0] + 1; // The first byte is the length of the rest of the header
                    if (udh_octet_len > ud_bytes.size()) {
                        // printf("[PDU Decode] UDH length (%d) exceeds available UD bytes (%zu).\n", udh_octet_len, ud_bytes.size());
                        udh_octet_len = 0; // Treat as no UDH
                        result.hasUserDataHeader = false;
                    } else {
                        // Check for standard UDH elements that might indicate MMS notification (WAP PUSH SI)
                        // This check is heuristic but matches common practice.
                        // WAP PUSH SI (Port 2948 or 9200-9203) UDH IE: 0x05 (App Port Addr), 0x04 (length), Dest Port (2 bytes), Orig Port (2 bytes)
                        if (udh_octet_len >= 6) { // UDH needs at least 1 (len) + 1 (IEI) + 1 (IEDL) + 4 (ports) = 7 bytes minimum
                             // Wait, the old code checked if udh_octet_len was >= 6 *after* finding 0x05 IE,
                             // but the UDH itself must be large enough to contain IEI, IEDL, and the data.
                             // The IE is 0x05, length is 0x04. Total IE data size is 1+1+4 = 6 bytes.
                             // The UDH header *format* is Length (1 byte) + IEI (1 byte) + IEDL (1 byte) + IEData (IEDL bytes) + ...
                             // So, a UDH containing only the App Port Addr IE is 1 (Total Len) + 1 (IEI) + 1 (IEDL=4) + 4 (Data) = 7 bytes.
                             // Let's iterate through the UDH IEs.
                             for (int ie_pos = 1; ie_pos < udh_octet_len; ) { // Start after the UDH length byte
                                 if (ie_pos + 1 >= udh_octet_len) break; // Need at least IEI and IEDL bytes
                                 unsigned char iei = ud_bytes[ie_pos];
                                 unsigned char iedl = ud_bytes[ie_pos+1];
                                 if (ie_pos + 1 + iedl >= udh_octet_len) { // Ensure IE data is within UDH bounds
                                     // printf("[PDU Decode] UDH IE length (%d) goes beyond UDH bounds (current_pos=%d, udh_len=%d).\n", iedl, ie_pos, udh_octet_len);
                                     break; // Stop processing UDH IEs
                                 }
    
                                 if (iei == 0x05 && iedl == 0x04) { // Application Port Addressing, 16-bit ports
                                    unsigned short dest_port = (ud_bytes[ie_pos+2] << 8) | ud_bytes[ie_pos+3];
                                    // Check common WAP Push/MMS ports
                                    if (dest_port == 2948 || dest_port == 9200 || dest_port == 9201 || dest_port == 9202 || dest_port == 9203) {
                                        result.type = SmsType::MMS_NOTIFICATION;
                                        // printf("[PDU Decode] Detected MMS Notification via Destination Port %hu.\n", dest_port);
                                        break; // Found the indicator, no need to check other IEs
                                    }
                                 }
                                 // Move to the next Information Element
                                 ie_pos += 2 + iedl;
                             }
                        }
    
                        // For 7-bit decoding, UDH length is needed in septets
                        if (is_7bit_encoding) {
                            // UDH length in octets (udh_octet_len) needs conversion to septet count for skipping
                            // The total number of septets used by UDH is ceil(udh_octet_len * 8.0 / 7.0)
                            udh_septet_len = static_cast<int>(std::ceil(udh_octet_len * 8.0 / 7.0));
                            // The actual TP-UDL includes UDH length in septets for 7-bit
                            // The total number of septets is tp_udl. Septets for UD = tp_udl - udh_septet_len.
                            // This calculation isn't strictly needed here, the unpacker just needs the offset.
                            // printf("[PDU Decode] UDH is %d octets, %d septets for 7bit offset.\n", udh_octet_len, udh_septet_len);
                        }
                    }
                }
    
                // Decode User Data based on encoding
                if (is_7bit_encoding) {
                     // For 7-bit, tp_udl is the TOTAL number of septets (including UDH).
                     // unpack7BitData needs the total septet count (tp_udl) and the UDH offset (udh_septet_len).
                    result.messageContent = unpack7BitData(ud_bytes, tp_udl, result.hasUserDataHeader ? udh_septet_len : 0);
                } else if (is_8bit_encoding) {
                     // For 8-bit, tp_udl is the TOTAL number of octets (including UDH).
                     // The actual message data starts after the UDH.
                    std::string msg_data_8bit;
                    for(size_t k = udh_octet_len; k < ud_bytes.size(); ++k) {
                        // As per old code's likely effect and for simplicity,
                        // directly append bytes as chars. Handle potential non-printable later if needed.
                        msg_data_8bit += static_cast<char>(ud_bytes[k]);
                    }
                    result.messageContent = msg_data_8bit;
                    // printf("[PDU Decode] Decoded 8-bit data (direct byte map).\n");
                } else if (is_ucs2_encoding) {
                     // For UCS-2, tp_udl is the TOTAL number of octets (including UDH).
                     // The actual message data starts after the UDH.
                     if ((ud_bytes.size() - udh_octet_len) % 2 != 0) {
                          // printf("[PDU Decode] UCS2 data length after UDH is odd. Data may be truncated.\n");
                     }
                     result.messageContent = ucs2BytesToUtf8(ud_bytes, udh_octet_len);
                     // printf("[PDU Decode] Decoded UCS-2 data to UTF-8.\n");
                } else {
                    // printf("[PDU Decode] Cannot decode User Data due to unknown encoding.\n");
                    return false;
                }
    
                // For MMS Notifications, extract URL from the decoded content (heuristic)
                if (result.type == SmsType::MMS_NOTIFICATION) {
                    // Search for "http" or "https" in the decoded message content
                    size_t http_pos = result.messageContent.find("http");
                    if (http_pos != std::string::npos) {
                        size_t url_end = result.messageContent.find_first_of(" \t\r\n\0", http_pos); // Find end of URL
                        if (url_end == std::string::npos) {
                            result.mmsUrl = result.messageContent.substr(http_pos);
                        } else {
                            result.mmsUrl = result.messageContent.substr(http_pos, url_end - http_pos);
                        }
                        // printf("[PDU Decode] Extracted potential MMS URL: %s\n", result.mmsUrl.c_str());
                    } else {
                        // printf("[PDU Decode] MMS Notification received, but no URL found starting with 'http'.\n");
                    }
                    // The message content for MMS notifications is often the raw WAP Push SI data,
                    // so keeping it or clearing it depends on need. Let's keep the decoded version.
                }
    
            } else if (tp_mti == 0x02) { // TP-STATUS-REPORT
                result.type = SmsType::STATUS_REPORT;
                // printf("[PDU Decode] SMS Status Report received (further parsing not implemented).\n");
                // Status reports are not messages to store/display as conversation text.
                // We can return true here if the structure is valid, even if full parsing isn't done.
                // Mandatory fields: TP-SCA (handled by SCA len), TP-MTI, TP-MR, TP-RA, TP-SCTS, TP-DT, TP-ST.
                // We've parsed up to SCTS. Need to skip DT and ST.
                if (current_pos + 7*2 + 2 > pdu_hex_string.length()) { // Skip TP-DT (7 octets) + TP-ST (1 octet)
                     // printf("[PDU Decode] PDU string too short for Status Report DT/ST fields.\n");
                     // Return false or try to handle partial? Let's return true if mandatory fields were mostly ok.
                     // For now, just acknowledging the type and returning true is sufficient based on the prompt.
                     // If more robust status report handling is needed, this part needs fleshing out.
                }
                return true;
    
            } else {
                // printf("[PDU Decode] Unhandled TP-MTI: 0x%02X\n", tp_mti);
                return false;
            }
    
            return true; // Parsing successful
        } catch (const std::exception& e) {
            // printf("[PDU Decode] Exception: %s. PDU: %s\n", e.what(), pdu_hex_string.c_str());
            return false;
        } catch (...) { // Catch any other unexpected errors
             // printf("[PDU Decode] Unknown Exception during PDU decoding. PDU: %s\n", pdu_hex_string.c_str());
             return false;
        }
    }

    static void processAndStoreSms(const std::string& pdu_hex_string, int message_idx) {
        // printf("[GSM SMS] Processing PDU for index %d: %s\n", message_idx, pdu_hex_string.c_str());
        DecodedPdu decoded_sms;
        if (decodePduDeliver(pdu_hex_string, decoded_sms)) {
            // printf("[GSM SMS] Decoded: Sender='%s', Type=%d, Message='%s'\n",
            //       decoded_sms.senderNumber.c_str(), static_cast<int>(decoded_sms.type), decoded_sms.messageContent.c_str());
            if (!decoded_sms.mmsUrl.empty()) {
                // printf("[GSM SMS] MMS URL: %s\n", decoded_sms.mmsUrl.c_str());
            }

            if (decoded_sms.type == SmsType::TEXT_SMS || decoded_sms.type == SmsType::MMS_NOTIFICATION) {
                if (decoded_sms.type == SmsType::MMS_NOTIFICATION) {
                    // printf("[GSM SMS] MMS Notification from: %s\n", decoded_sms.senderNumber.c_str());
                    if (!decoded_sms.mmsUrl.empty()) {
                        // printf("[GSM SMS] MMS URL found: %s. Triggering getHttpMMS (placeholder).\n", decoded_sms.mmsUrl.c_str());
                        // Your actual call: getHttpMMS(decoded_sms.senderNumber, decoded_sms.mmsUrl);
                    } else {
                         // printf("[GSM SMS] MMS Notification, but no URL extracted.\n");
                    }
                } else { 
                    // printf("[GSM SMS] Text SMS from: %s, Message: %s\n", decoded_sms.senderNumber.c_str(), decoded_sms.messageContent.c_str());
                    
                    auto contact = Contacts::getByNumber("+" + decoded_sms.senderNumber);

                    // Ajout du message à la conversation
                    Conversations::Conversation conv;
                    storage::Path convPath(std::string(MESSAGES_LOCATION) + "/+" + decoded_sms.senderNumber + ".json");
                    if (convPath.exists())
                    {
                        Conversations::loadConversation(convPath, conv);
                    }
                    else
                    {
                        conv.number = decoded_sms.senderNumber;
                    }

                    conv.messages.push_back({decoded_sms.messageContent, true, std::to_string(getCurrentTimestamp())}); // true = message de l'autre
                    Conversations::saveConversation(convPath, conv);

                    storage::FileStream file(std::string(MESSAGES_NOTIF_LOCATION), storage::Mode::READ);
                    std::string content = file.read();
                    file.close();

                    if(content.find("+" + decoded_sms.senderNumber) == std::string::npos)
                    {
                        storage::FileStream file2(storage::Path(std::string(MESSAGES_NOTIF_LOCATION)).str(), storage::Mode::APPEND);
                        file2.write("+" + decoded_sms.senderNumber + "\n");
                        file2.close();
                    }
                }
                if (ExternalEvents::onNewMessage) {
                    ExternalEvents::onNewMessage();
                }
            }

            auto delete_req = std::make_shared<Request>();
            delete_req->command = "AT+CMGD=" + std::to_string(message_idx);
            delete_req->callback = [message_idx](const std::string& response) -> bool {
                if (response.find("OK") != std::string::npos) {
                    // printf("[GSM SMS] Successfully deleted SMS at index %d.\n", message_idx);
                } else {
                    // printf("[GSM SMS] Failed to delete SMS at index %d. Response: %s\n", message_idx, response.c_str());
                }
                return false; 
            };
            requests.push_back(delete_req);

        } else {
            // printf("[GSM SMS] Failed to decode PDU for message at index %d.\n", message_idx);
        }
    }

    // Forward declarations for helper functions
    bool isEndIdentifier(const std::string &data);
    bool isURC(const std::string &data);
    // end of forward declarations

    static void queueReadSms(const std::string& memory_store, int index) {
        // printf("[GSM SMS] Queuing read for SMS from %s at index %d.\n", memory_store.c_str(), index);
        auto request = std::make_shared<Request>();
        request->command = "AT+CMGR=" + std::to_string(index);
        request->callback = [index](const std::string& response_block) -> bool {
            std::string pdu_line;
            bool found_pdu = false;
            std::stringstream ss_block(response_block);
            std::string line;
            bool cmgr_header_found = false;

            while (std::getline(ss_block, line, '\n')) {
                if (!line.empty() && line.back() == '\r') line.pop_back(); 
                line.erase(0, line.find_first_not_of(" \t")); 
                line.erase(line.find_last_not_of(" \t") + 1); 
                if (line.empty()) continue;

                if (line.rfind("+CMGR:", 0) == 0) {
                    cmgr_header_found = true;
                } else if (cmgr_header_found && line != "OK" && line.rfind("ERROR", 0) != 0 && !isEndIdentifier(line) && !isURC(line)) {
                    pdu_line = line;
                    found_pdu = true;
                    break; 
                }
            }

            if (found_pdu) {
                processAndStoreSms(pdu_line, index);
            } else {
                // printf("[GSM SMS] Failed to extract PDU from AT+CMGR response for index %d. Block:\n%s\n", index, response_block.c_str());
            }
            return false; 
        };
        requests.push_back(request);
    }

    void checkForMessages() {
        // printf("[GSM SMS] Queuing check for all stored messages (AT+CMGL=0).\n");
        auto request = std::make_shared<Request>();
        // AT+CMGL=0: List ALL messages.
        // If CMGF=0 (PDU mode), this should list the PDU data directly for each message.
        // Some modules also accept AT+CMGL="ALL" which is equivalent to 4.
        request->command = "AT+CMGL=0"; // Or "AT+CMGL=\"ALL\"" if your module prefers
    
        request->callback = [](const std::string& response_block) -> bool {
            std::stringstream ss_block(response_block);
            std::string line;
            int messages_found_and_processed = 0;
            bool command_ok = response_block.find("OK") != std::string::npos;
            // Note: An ERROR here is more significant if we're trying to list ALL messages.
            bool command_error = response_block.find("ERROR") != std::string::npos;
    
            int current_message_idx = -1;
            // std::string current_pdu_line; // Not strictly needed if PDU is on one line
            bool expect_pdu_next = false;
    
            while (std::getline(ss_block, line, '\n')) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);
                if (line.empty()) continue;
    
                if (line.rfind("+CMGL:", 0) == 0) {
                    // Expected format for +CMGL in PDU mode (CMGF=0):
                    // +CMGL: <index>,<stat_val>,[<alpha>],<length_of_PDU><CR><LF><PDU_data>
                    // Example:
                    // +CMGL: 1,0,,17  (<stat_val> 0 for REC UNREAD, 1 for REC READ etc.)
                    // 0791... (PDU data)
                    std::string data_part = line.substr(6); // Skip "+CMGL: "
                    size_t first_comma = data_part.find(',');
                    if (first_comma != std::string::npos) {
                        std::string index_str = data_part.substr(0, first_comma);
                        index_str.erase(0, index_str.find_first_not_of(" \t"));
                        index_str.erase(index_str.find_last_not_of(" \t") + 1);
                        try {
                            current_message_idx = std::stoi(index_str);
                            expect_pdu_next = true;
                            // current_pdu_line = ""; // Reset if accumulating multi-line PDUs
                            // printf("[GSM SMS] Found +CMGL header for index %d. Expecting PDU.\n", current_message_idx);
                        } catch (const std::invalid_argument& ia) {
                            // printf("[GSM SMS] Failed to parse index (invalid_argument) from +CMGL line: '%s'. Line: '%s'\n", index_str.c_str(), line.c_str());
                            expect_pdu_next = false;
                            current_message_idx = -1;
                        } catch (const std::out_of_range& oor) {
                            // printf("[GSM SMS] Failed to parse index (out_of_range) from +CMGL line: '%s'. Line: '%s'\n", index_str.c_str(), line.c_str());
                            expect_pdu_next = false;
                            current_message_idx = -1;
                        }
                    } else {
                        // printf("[GSM SMS] Malformed +CMGL line (no comma for index): %s\n", line.c_str());
                        expect_pdu_next = false;
                        current_message_idx = -1;
                    }
                } else if (expect_pdu_next && current_message_idx != -1) {
                    // This line should be the PDU data.
                    std::string pdu_data_line = line;
                    // printf("[GSM SMS] Received PDU for index %d: %s\n", current_message_idx, pdu_data_line.c_str());
                    processAndStoreSms(pdu_data_line, current_message_idx); // This will also queue deletion
                    messages_found_and_processed++;
                    expect_pdu_next = false;
                    current_message_idx = -1;
                } else if (isEndIdentifier(line) || isURC(line)) {
                    if (expect_pdu_next) {
                        // printf("[GSM SMS] Expected PDU data for index %d, but received '%s' instead.\n", current_message_idx, line.c_str());
                        expect_pdu_next = false;
                        current_message_idx = -1;
                    }
                }
            }
    
            if (command_ok) {
                if (messages_found_and_processed > 0) {
                    // printf("[GSM SMS] AT+CMGL=0 successful. Processed %d messages.\n", messages_found_and_processed);
                } else {
                    // This is normal if there are no messages at all.
                    // printf("[GSM SMS] AT+CMGL=0 successful. No messages found in storage.\n");
                }
            } else if (command_error) {
                // An ERROR with AT+CMGL=0 is more concerning.
                // printf("[GSM SMS] AT+CMGL=0 command returned ERROR. Module may not support it or other issue.\n");
                reboot();
            } else {
                if (messages_found_and_processed > 0) {
                     // printf("[GSM SMS] AT+CMGL=0 response did not explicitly contain OK/ERROR, but %d messages were processed. Block:\n%s\n", messages_found_and_processed, response_block.c_str());
                } else {
                     // printf("[GSM SMS] AT+CMGL=0 response did not contain OK/ERROR and no messages found/processed. Block:\n%s\n", response_block.c_str());
                }
            }
            return false;
        };
        requests.push_back(request);
    }

    // --- Internal Update Functions (Network, Voltage, etc.) ---
    static void updateNetworkQualityInternal() {
        auto request = std::make_shared<Request>();
        request->command = "AT+CSQ";
        request->callback = [](const std::string& response) -> bool {
            int rssi = 99;
            int ber = 99;
            bool success = false;
            auto pos = response.find("+CSQ:");
            if (pos != std::string::npos) {
                std::string values = response.substr(pos + 5);
                std::stringstream ss(values);
                char comma;
                ss >> std::ws >> rssi >> std::ws >> comma >> std::ws >> ber;
                if (!ss.fail() && comma == ',') {
                    success = true;
                }
            }
            if (success) {
                currentRssi = rssi;
                currentBer = ber;
                networkQualityValid = true;
                lastQualityUpdateTime = std::chrono::steady_clock::now();
                // printf("[GSM State] Updated Network Quality: RSSI=%d, BER=%d\n", currentRssi, currentBer);
            } else {
                 networkQualityValid = false; 
                 // printf("[GSM State] Failed to update Network Quality.\n");
            }
            return false; 
        };
        requests.push_back(request);
    }

    static void updateGprsAttachmentStatusInternal() {
        auto request = std::make_shared<Request>();
        request->command = "AT+CGATT?";
        request->callback = [](const std::string& response) -> bool {
            bool success = false;
            bool attached = false;
            auto pos = response.find("+CGATT:");
            if (pos != std::string::npos) {
                 std::string value_str = response.substr(pos + 7);
                 value_str.erase(0, value_str.find_first_not_of(" \t\r\n"));
                 if (!value_str.empty()) {
                     try {
                         int status = std::stoi(value_str);
                         attached = (status == 1);
                         success = true; 
                     } catch (...) {
                          // printf("[GSM State] Failed to parse CGATT status value: %s\n", value_str.c_str());
                          success = false; 
                     }
                 } else { success = false; } 
            } else if (response.find("OK") != std::string::npos && response.find("ERROR") == std::string::npos) {
                 attached = false;
                 success = true; 
                 // printf("[GSM State] GPRS Status: OK received but no +CGATT line. Assuming detached.\n");
            } else if (response.find("ERROR") != std::string::npos) {
                 attached = false;
                 success = true; 
                 // printf("[GSM State] GPRS Status: Command Error (Expected without SIM/Network).\n");
            }

            if (success) {
                isGprsAttached = attached;
                gprsStateValid = true;
                lastGprsUpdateTime = std::chrono::steady_clock::now();
                // printf("[GSM State] Updated GPRS Status: %s\n", isGprsAttached ? "Attached" : "Detached");
            } else {
                 gprsStateValid = false; 
                 // printf("[GSM State] Failed to update GPRS Status.\n");
            }
            return false;
        };
        requests.push_back(request);
    }

    static void updateFlightModeStatusInternal() {
        auto request = std::make_shared<Request>();
        request->command = "AT+CFUN?";
        request->callback = [](const std::string& response) -> bool {
            bool success = false;
            bool flightMode = false; 
            auto pos = response.find("+CFUN:");
            if (pos != std::string::npos) {
                std::string value = response.substr(pos + 6);
                 value.erase(0, value.find_first_not_of(" \t\r\n"));
                 value.erase(value.find_last_not_of(" \t\r\n") + 1);
                try {
                    int funLevel = std::stoi(value);
                    if (funLevel == 4 || funLevel == 0) {
                        flightMode = true;
                    } else { 
                        flightMode = false;
                    }
                    success = true;
                } catch (...) {
                     // printf("[GSM State] Failed to parse CFUN level: %s\n", value.c_str());
                }
            }
            if (success) {
                flightModeState = flightMode;
                flightModeStateValid = true;
                lastFlightModeUpdateTime = std::chrono::steady_clock::now();
                // printf("[GSM State] Updated Flight Mode Status: %s\n", flightModeState ? "ON (RF OFF)" : "OFF (RF ON)");
            } else {
                 flightModeStateValid = false; 
                 // printf("[GSM State] Failed to update Flight Mode Status.\n");
            }
            return false;
        };
        requests.push_back(request);
    }

    static void updateVoltageInternal() {
        auto request = std::make_shared<Request>();
        request->command = "AT+CBC";
        request->callback = [](const std::string& response) -> bool {
            std::stringstream ss_block(response);
            std::string line;
            bool found_cbc_line = false;
            bool parsed_value_ok = false;
            bool cmd_success = (response.find("OK") != std::string::npos);
            int voltage = -1; 

            while (std::getline(ss_block, line, '\n')) {
                 if (!line.empty() && line.back() == '\r') line.pop_back();
                 line.erase(0, line.find_first_not_of(" \t"));
                 line.erase(line.find_last_not_of(" \t") + 1);

                 if (line.rfind("+CBC:", 0) == 0) {
                    found_cbc_line = true; 
                    std::string data_part = line.substr(5); 
                    data_part.erase(0, data_part.find_first_not_of(" \t")); 

                    size_t first_comma = data_part.find(',');
                    size_t second_comma = std::string::npos;
                    if (first_comma != std::string::npos) {
                        second_comma = data_part.find(',', first_comma + 1);
                    }
                    std::string voltage_str_raw;
                    if (first_comma != std::string::npos && second_comma != std::string::npos) {
                        voltage_str_raw = data_part.substr(second_comma + 1);
                    } else {
                        voltage_str_raw = data_part;
                    }
                    voltage_str_raw.erase(std::remove_if(voltage_str_raw.begin(), voltage_str_raw.end(),
                                                        [](char c) { return !std::isdigit(c); }),
                                        voltage_str_raw.end());
                    try {
                        if (!voltage_str_raw.empty()) {
                            voltage = std::stoi(voltage_str_raw); 
                            parsed_value_ok = true; 
                        } else {
                             // printf("[GSM State] Voltage Line Found but voltage string was empty after cleaning. (Original Data Part: '%s')\n", data_part.c_str());
                        }
                    } catch (const std::invalid_argument& ia) {
                        // printf("[GSM State] Voltage Line Found but failed to parse numeric voltage value from cleaned string '%s' (Original Data Part: '%s')\n",
                        //       voltage_str_raw.c_str(), data_part.c_str());
                    } catch (const std::out_of_range& oor) {
                        // printf("[GSM State] Voltage Line Found but numeric value out of range from cleaned string '%s' (Original Data Part: '%s')\n",
                        //       voltage_str_raw.c_str(), data_part.c_str());
                    }
                    break; 
                 }
            }
            bool update_successful = false;
            if (found_cbc_line && parsed_value_ok) { 
                currentVoltage_mV = voltage;
                try {
                    battery_voltage_history.push_back(currentVoltage_mV);
                    if (battery_voltage_history.size() > 24)
                        battery_voltage_history.erase(battery_voltage_history.begin());
                    if (battery_voltage_history.size() > 0) {
                        double sum = 0;
                        for (auto v : battery_voltage_history)
                            sum += v;
                            currentVoltage_mV = sum / battery_voltage_history.size();
                        // std::cout << "[GSM State] Battery voltage average: " << currentVoltage_mV << std::endl; // Corrected variable
                    }
                } catch (std::exception) { }

                voltageValid = true;
                lastVoltageUpdateTime = std::chrono::steady_clock::now();
                // printf("[GSM State] Updated Voltage: %d mV\n", currentVoltage_mV);
                update_successful = true;
            } else if (!cmd_success && response.find("ERROR") != std::string::npos) {
                 // printf("[GSM State] Voltage Update: Command Error received.\n");
            } else if (cmd_success && !found_cbc_line) {
                 // printf("[GSM State] Voltage Update: OK received but +CBC line not found.\n");
            } else if (cmd_success && found_cbc_line && !parsed_value_ok) {
                 // printf("[GSM State] Voltage Update: OK received, +CBC line found, but failed to parse voltage value.\n");
            } else if (!cmd_success && !found_cbc_line) {
                 // printf("[GSM State] Voltage Update: Unexpected response state (No OK/ERROR/CBC).\n");
            }

            if (!update_successful) {
                 voltageValid = false; 
                 // printf("[GSM State] Failed to update Voltage state this cycle.\n");
            }
            return false; 
        };
        requests.push_back(request);
    }

    double getBatteryLevel() {
#ifdef ESP_PLATFORM
        if (currentVoltage_mV == -1) {
            return 1; // Or 0, or some other indicator of unknown
        }
        const double voltage_V = currentVoltage_mV / 1000.0;
        const double batteryLevel = 3.083368 * std::pow(voltage_V, 3) - 37.21203 * std::pow(voltage_V, 2) + 150.5735 * voltage_V - 203.3347;
        return std::clamp(batteryLevel, 0.0, 1.0);
#else
        return 1; // Default for non-ESP platforms
#endif
    }

    static void updatePinStatusInternal() {
        auto request = std::make_shared<Request>();
        request->command = "AT+CPIN?";
        request->callback = [](const std::string& response) -> bool {
            std::stringstream ss_block(response);
            std::string line;
            std::string parsed_status = "";
            bool found_cpin_line = false;
            bool cmd_success = (response.find("OK") != std::string::npos);

            while (std::getline(ss_block, line, '\n')) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);
                if (line.rfind("+CPIN:", 0) == 0) {
                    parsed_status = line.substr(6);
                    parsed_status.erase(0, parsed_status.find_first_not_of(" \t"));
                    parsed_status.erase(parsed_status.find_last_not_of(" \t") + 1);
                    found_cpin_line = true;
                    break;
                }
            }
            bool needsPin = true; 
            bool status_determined = false;

            if (found_cpin_line) {
                if (parsed_status == "READY") {
                    needsPin = false;
                    status_determined = true;
                } else if (parsed_status == "SIM PIN" || parsed_status == "SIM PUK") {
                    needsPin = true;
                    status_determined = true;
                } else if (parsed_status == "NOT INSERTED" || parsed_status.find("NO SIM") != std::string::npos || parsed_status.find("NOT READY") != std::string::npos) {
                    needsPin = true; 
                    status_determined = true;
                    // printf("[GSM State] PIN Status: SIM Not Inserted or Not Ready.\n");
                } else {
                    needsPin = true; 
                    status_determined = true;
                    // printf("[GSM State] PIN Status: Unknown Module Status (%s)\n", parsed_status.c_str());
                }
            } else if (!cmd_success && response.find("ERROR") != std::string::npos) {
                 // printf("[GSM State] PIN Status: Command Error received.\n");
                 needsPin = true;
                 status_determined = false; 
            } else if (cmd_success && !found_cpin_line) {
                 // printf("[GSM State] PIN Status: OK received but no +CPIN line found.\n");
                 needsPin = true; 
                 status_determined = false;
            } else {
                 // printf("[GSM State] PIN Status: Unexpected response state.\n");
                 needsPin = true;
                 status_determined = false;
            }

            if (status_determined) {
                pinRequiresPin = needsPin;
                pinStatusValid = true;
                lastPinStatusUpdateTime = std::chrono::steady_clock::now();
                // printf("[GSM State] Updated PIN Status: %s\n", pinRequiresPin ? "Required/Not Ready" : "Ready");
            } else {
                 pinStatusValid = false; 
                 // printf("[GSM State] Failed to update PIN Status (Parsing/Command Error or Unknown State).\n");
            }
            return false; 
        };
        requests.push_back(request);
    }

    static void updatePduModeStatusInternal() {
        auto request = std::make_shared<Request>();
        request->command = "AT+CMGF?";
        request->callback = [](const std::string& response) -> bool {
            bool success = false;
            bool pduMode = false; 
            auto pos = response.find("+CMGF:");
            if (pos != std::string::npos) {
                std::string value = response.substr(pos + 6);
                 value.erase(0, value.find_first_not_of(" \t\r\n"));
                 value.erase(value.find_last_not_of(" \t\r\n") + 1);
                try {
                    int mode = std::stoi(value);
                    if (mode == 0) { 
                        pduMode = true;
                    } else { 
                        pduMode = false;
                    }
                    success = true;
                } catch (...) {
                     // printf("[GSM State] Failed to parse CMGF mode: %s\n", value.c_str());
                }
            }
            if (success) {
                pduModeEnabled = pduMode;
                pduModeStateValid = true;
                lastPduModeUpdateTime = std::chrono::steady_clock::now();
                // printf("[GSM State] Updated PDU Mode Status: %s\n", pduModeEnabled ? "Enabled (0)" : "Disabled (1)");
            } else {
                 pduModeStateValid = false; 
                 // printf("[GSM State] Failed to update PDU Mode Status.\n");
            }
            return false;
        };
        requests.push_back(request);
    }


    // --- Initialization ---
    void init()
    {
        #ifdef ESP_PLATFORM
        // printf("[GSM] Initializing...\n");
        pinMode(GSM_PWR_PIN, OUTPUT);
        digitalWrite(GSM_PWR_PIN, 0); 
        gsm.setRxBufferSize(4096); 

        // printf("[GSM] Ensuring module is off before power cycle...\n");
        digitalWrite(GSM_PWR_PIN, 1); 
        delay(1500); 
        digitalWrite(GSM_PWR_PIN, 0); 
        delay(3000); 

        // printf("[GSM] Powering on module...\n");
        digitalWrite(GSM_PWR_PIN, 1); 
        delay(1200); 
        digitalWrite(GSM_PWR_PIN, 0); 
        // printf("[GSM] Module power sequence complete. Waiting for boot (10s)...\n");
        delay(10000); 


        bool comm_ok = false;
        int attempts = 0;
        const int max_attempts = 3; 

        // printf("[GSM] Attempting communication at 921600 bps...\n");
        gsm.begin(921600, SERIAL_8N1, RX, TX);
        delay(100); 

        while(attempts < max_attempts && !comm_ok) {
            gsm.println("AT");
            delay(500); 
            String data = "";
            while(gsm.available()) { data += (char)gsm.read(); } 
            Serial.print("[GSM RX @921600]: "); Serial.println(data);

            if (data.indexOf("OK") != -1) {
                // printf("[GSM] Communication established at 921600 bps.\n");
                comm_ok = true;
            } else {
                // printf("[GSM] No valid response at 921600. Retrying...\n");
                delay(2000); 
            }
            attempts++;
        }

        if (!comm_ok) {
            // printf("[GSM] Failed at 921600. Trying 115200 bps...\n");
            gsm.updateBaudRate(115200); 
            delay(100);
            attempts = 0;
             while(attempts < max_attempts && !comm_ok) {
                gsm.println("AT");
                delay(500);
                String data = "";
                while(gsm.available()) { data += (char)gsm.read(); }
                Serial.print("[GSM RX @115200]: "); Serial.println(data);

                if (data.indexOf("OK") != -1) {
                    // printf("[GSM] Communication established at 115200 bps. Setting preferred rate (921600)...\n");
                    gsm.println("AT+IPR=921600"); 
                    delay(100); gsm.flush(); 
                    gsm.updateBaudRate(921600); 
                    delay(100);
                    gsm.println("AT"); delay(500); data = ""; while(gsm.available()) { data += (char)gsm.read(); }
                    Serial.print("[GSM RX @921600]: "); Serial.println(data);
                    if (data.indexOf("OK") != -1) {
                         // printf("[GSM] Communication verified at 921600.\n");
                         comm_ok = true;
                    } else {
                         // printf("[GSM] Failed to verify communication after setting 921600. Sticking with 115200.\n");
                         gsm.updateBaudRate(115200);
                         delay(100);
                         comm_ok = true; 
                    }
                } else {
                    // printf("[GSM] No valid response at 115200. Retrying...\n");
                    delay(2000);
                }
                attempts++;
            }
        }


        if (comm_ok) {
             // printf("[GSM] Module Initialized.\n");
             currentCallState = CallState::IDLE; 

             // printf("[GSM] Queuing initial state checks...\n");
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

        // printf("[GSM] Initialization failed after all attempts.\n");
        currentCallState = CallState::UNKNOWN; 
        #else
        // printf("[GSM] Dummy Init Completed.\n");
        currentCallState = CallState::IDLE;
        #endif
    }

    void reboot()
    {
        auto req = std::make_shared<Request>();
        req->command = "AT+CFUN=1,1";
        req->callback = [](const std::string& response) -> bool {
            init();
            return false;
        };
        // printf("[GSM] Rebooting module...\n");
        requests.push_back(req);
    }

    void uploadSettings()
    {
        // printf("[GSM] Configuring module settings (URCs)...\n");

        auto requestClip = std::make_shared<Request>();
        requestClip->command = "AT+CLIP=1";
        requestClip->callback = [](const std::string& response) -> bool {
            // printf("[GSM Settings] CLIP %s.\n", (response.find("OK") != std::string::npos) ? "enabled" : "failed");
            return false;
        };
        requests.push_back(requestClip);

        auto requestClcc = std::make_shared<Request>();
        requestClcc->command = "AT+CLCC=1";
        requestClcc->callback = [](const std::string& response) -> bool {
            // printf("[GSM Settings] CLCC URC %s.\n", (response.find("OK") != std::string::npos) ? "enabled" : "failed");
            return false;
        };
        requests.push_back(requestClcc);

        auto requestCnmi = std::make_shared<Request>();
        requestCnmi->command = "AT+CNMI=2,1,0,0,0";
        requestCnmi->callback = [](const std::string& response) -> bool {
            // printf("[GSM Settings] CNMI %s.\n", (response.find("OK") != std::string::npos) ? "configured" : "failed");
            return false;
        };
        requests.push_back(requestCnmi);

        auto requestHourSync = std::make_shared<Request>();
        requestHourSync->command = "AT+CNTP=\"time.google.com\",8";
        requestHourSync->callback = [](const std::string& response) -> bool {
            // printf("[GSM Settings] Hour Sync %s.\n", (response.find("OK") != std::string::npos) ? "configured" : "failed");
            return false;
        };
        requests.push_back(requestHourSync);

        auto requestMinuteSync = std::make_shared<Request>();
        requestMinuteSync->command = "AT+CNTP";
        requestMinuteSync->callback = [](const std::string& response) -> bool {
            // printf("[GSM Settings] Hour Sync Status: %s.\n", response.c_str());
            return false;
        };
        requests.push_back(requestMinuteSync);

        auto requestCreg = std::make_shared<Request>();
        requestCreg->command = "AT+CREG=1";
        requestCreg->callback = [](const std::string& response) -> bool {
             // printf("[GSM Settings] CREG URC %s.\n", (response.find("OK") != std::string::npos) ? "enabled" : "failed");
             return false;
        };
        requests.push_back(requestCreg);

        auto requestCgreg = std::make_shared<Request>();
        requestCgreg->command = "AT+CGREG=1";
        requestCgreg->callback = [](const std::string& response) -> bool {
             // printf("[GSM Settings] CGREG URC %s.\n", (response.find("OK") != std::string::npos) ? "enabled" : "failed");
             if (response.find("OK") != std::string::npos) {
                 updateGprsAttachmentStatusInternal();
             }
             return false;
        };
        requests.push_back(requestCgreg);

        // Ensure PDU mode is set for SMS processing
        // After setting PDU mode, check for any stored messages
        setPduMode(true, [](bool success){
            if(success) {
                // printf("[GSM Settings] PDU Mode (CMGF=0) set successfully for SMS.\n");
                // After setting PDU mode, check for any stored messages
                checkForMessages(); // <<< ADDED CALL HERE
            }
            else {} // printf("[GSM Settings] Failed to set PDU Mode (CMGF=0) for SMS.\n");
        });
    }

    bool isEndIdentifier(const std::string& data) {
        if (data == "OK" || data == "ERROR" || data == "NO CARRIER" ||
            data == "BUSY" || data == "NO ANSWER" || data == "NO DIALTONE") return true;
        if (data.rfind("+CME ERROR:", 0) == 0 || data.rfind("+CMS ERROR:", 0) == 0) return true;
        return false;
    }

    const std::vector<std::string> known_urc_prefixes = {
        "RING", "+CLIP:", "+CLCC:", "NO CARRIER", "BUSY", "NO ANSWER",
        "+CMTI:", "+CMT:", "+CBM:", "+CDS:", "+CREG:", "+CGREG:", "+CEREG:",
        "+PDP:", "+SAPBR:", "Call Ready", "SMS Ready", "NORMAL POWER DOWN",
        "UNDER-VOLTAGE WARNING", "UNDER-VOLTAGE POWER DOWN", "+SIMREADY",
        "+RECEIVE", "CLOSED", "REMOTE IP:", "+IP_INITIAL", "+IP_STATUS",
        "+PDP_DEACT", "+SAPBR_DEACT", "+LOCATION:", "+CIPGSMLOC:",
        "+HTTPACTION:", "+FTPGET:", "+CGEV:"
    };

    bool isURC(const std::string& data) {
        if (data.empty()) return false;
        for (const std::string& prefix : known_urc_prefixes) {
            if (data.find(prefix) == 0) return true;
        }
        return false;
    }

    void processURC(std::string data)
    {
        // printf("[GSM URC]: %s\n", data.c_str());

        if (data.find("RING") == 0) {
            currentCallState = CallState::RINGING;
            lastIncomingCallNumber = ""; 
        } else if (data.find("+CLIP:") == 0) {
            size_t firstQuote = data.find('"');
            size_t secondQuote = data.find('"', firstQuote + 1);
            if (firstQuote != std::string::npos && secondQuote != std::string::npos) {
                lastIncomingCallNumber = data.substr(firstQuote + 1, secondQuote - firstQuote - 1);
                // printf("[GSM State] Incoming call from: %s\n", lastIncomingCallNumber.c_str());
                if(ExternalEvents::onIncommingCall)
                    ExternalEvents::onIncommingCall();
                if(currentCallState != CallState::RINGING) {
                    currentCallState = CallState::RINGING;
                }
            }
        } else if (data.rfind("+HTTPACTION:", 0) == 0) {
            if (currentHttpState != HttpState::ACTION_IN_PROGRESS) return;

            int statusCode = 0, dataLen = 0;
            sscanf(data.c_str(), "+HTTPACTION: %*d,%d,%d", &statusCode, &dataLen);

            // printf("[GSM HTTP] Action URC. Status: %d, Length: %d\n", statusCode, dataLen);

            if (statusCode >= 200 && statusCode < 300) {
                if (dataLen > 0) {
                    httpBytesTotal = dataLen;
                    httpBytesRead = 0;
                    currentHttpState = HttpState::READING;
                    _queueNextHttpRead(); 
                } else {
                    _completeHttpRequest(HttpResult::OK);
                }
            } else if (statusCode == 404) {
                _completeHttpRequest(HttpResult::NOT_FOUND);
            } else if (statusCode >= 400) {
                _completeHttpRequest(HttpResult::SERVER_ERROR);
            } else {
                _completeHttpRequest(HttpResult::CONNECTION_FAILED);
            }
        } else if (data.find("+CLCC:") == 0) {
            std::stringstream ss_clcc(data.substr(6)); 
            std::string clcc_part;
            std::vector<std::string> clcc_parts;
            while(std::getline(ss_clcc, clcc_part, ',')) { clcc_parts.push_back(clcc_part); }

            if (clcc_parts.size() >= 3) {
                try {
                    int stat = std::stoi(clcc_parts[2]);
                    CallState previousState = currentCallState;
                    switch(stat) {
                        case 0: currentCallState = CallState::ACTIVE; break; 
                        case 2: currentCallState = CallState::DIALING; break; 
                        case 3: currentCallState = CallState::DIALING; break; 
                        case 4: currentCallState = CallState::RINGING; break; 
                        case 6: currentCallState = CallState::IDLE; break; 
                        default: break; 
                    }
                     if (currentCallState != previousState) {
                        // printf("[GSM State] Call state updated via CLCC: %d -> %d\n", static_cast<int>(previousState), static_cast<int>(currentCallState));
                        if (currentCallState == CallState::IDLE) {
                            lastIncomingCallNumber = ""; 
                        }
                     }
                } catch (...) {} // printf("[GSM URC] Failed to parse CLCC status.\n"); }
            }
             if (data == "+CLCC:") { 
                 if (currentCallState != CallState::IDLE) {
                     // printf("[GSM State] Empty +CLCC URC received, assuming IDLE.\n");
                     currentCallState = CallState::IDLE;
                     lastIncomingCallNumber = "";
                 }
             }

        } else if (data == "NO CARRIER" || data == "BUSY" || data == "NO ANSWER") {
            if (currentCallState != CallState::IDLE) {
                 // printf("[GSM State] Call terminated/failed URC (%s). Setting state to IDLE.\n", data.c_str());
                 currentCallState = CallState::IDLE;
                 lastIncomingCallNumber = "";
            }
        }
        else if (data.find("+CMTI:") == 0) { 
            // printf("[GSM URC] New SMS received indication: %s\n", data.c_str());
            size_t first_quote = data.find('"');
            size_t second_quote = data.find('"', first_quote + 1);
            size_t comma_after_quote = data.find(',', second_quote +1);

            if (first_quote != std::string::npos && 
                second_quote != std::string::npos && 
                comma_after_quote != std::string::npos) 
            {
                std::string mem_store = data.substr(first_quote + 1, second_quote - first_quote - 1);
                std::string index_str = data.substr(comma_after_quote + 1);
                index_str.erase(0, index_str.find_first_not_of(" \t"));
                index_str.erase(index_str.find_last_not_of(" \t") + 1);
                try {
                    int msg_idx = std::stoi(index_str);
                    queueReadSms(mem_store, msg_idx);
                } catch (const std::exception& e) {
                    // printf("[GSM URC] Failed to parse +CMTI index: %s. Data: %s\n", e.what(), data.c_str());
                }
            } else {
                 // printf("[GSM URC] Failed to parse +CMTI format. Data: %s\n", data.c_str());
            }
        }
        else if (data.find("+CREG:") == 0 || data.find("+CGREG:") == 0) {
            // printf("[GSM URC] Network registration URC: %s\n", data.c_str());
            size_t comma_pos = data.find(',');
            if (comma_pos != std::string::npos) {
                try {
                    std::string stat_str = data.substr(comma_pos + 1);
                    size_t next_comma = stat_str.find(',');
                    if(next_comma != std::string::npos) { stat_str = stat_str.substr(0, next_comma); }
                    stat_str.erase(0, stat_str.find_first_not_of(" \t"));
                    int stat = std::stoi(stat_str);
                    bool attached = (stat == 1 || stat == 5); 

                    if (data.find("+CGREG:") == 0) { 
                        if (!gprsStateValid || isGprsAttached != attached) {
                             // printf("[GSM State] GPRS state updated via URC: %s (Stat=%d)\n", attached ? "Attached" : "Detached", stat);
                             isGprsAttached = attached;
                             gprsStateValid = true; 
                             lastGprsUpdateTime = std::chrono::steady_clock::now();
                        }
                    }
                } catch (...) {} // printf("[GSM URC] Failed to parse REG status.\n"); }
            }
        }
        else if (data == "Call Ready" || data == "SMS Ready" || data == "+SIMREADY") {
            // printf("[GSM URC] Module Ready URC: %s\n", data.c_str());
            if (currentCallState == CallState::UNKNOWN) {
                currentCallState = CallState::IDLE;
            }
            if (!pinStatusValid) {
                updatePinStatusInternal();
            }
        }
        else if (data.find("+CGEV:") == 0) {
             // printf("[GSM URC] GPRS/EPS Event: %s\n", data.c_str());
             if (data.find("DETACH") != std::string::npos) {
                 if (isGprsAttached) {
                     // printf("[GSM State] GPRS detached via CGEV URC.\n");
                     isGprsAttached = false;
                     gprsStateValid = true; 
                     lastGprsUpdateTime = std::chrono::steady_clock::now();
                 }
             }
        }
    }


    // --- Public Accessors (Getters) ---
    std::pair<int, int> getNetworkQuality() { return {currentRssi, currentBer}; }
    bool isNetworkQualityValid() { return networkQualityValid; }
    bool isConnected() { return isGprsAttached; }
    bool isConnectedStateValid() { return gprsStateValid; }
    bool isFlightModeActive() { return flightModeState; }
    bool isFlightModeStateValid() { return flightModeStateValid; }
    int getVoltage() { return currentVoltage_mV; }
    bool isVoltageValid() { return voltageValid; }
    bool isPinRequired() { return pinRequiresPin; }
    bool isPinStatusValid() { return pinStatusValid; }
    bool isPduModeEnabled() { return pduModeEnabled; }
    bool isPduModeStateValid() { return pduModeStateValid; }
    CallState getCallState() { return currentCallState; }
    std::string getLastIncomingNumber() { return lastIncomingCallNumber; }


    // --- Public Action Functions ---
    void setPin(const std::string& pin, std::function<void(bool success)> completionCallback) {
        auto request = std::make_shared<Request>();
        request->command = "AT+CPIN=" + pin;
        request->callback = [completionCallback](const std::string& response) -> bool {
            bool success = (response.find("OK") != std::string::npos);
            // printf("[GSM Action] setPin %s.\n", success ? "succeeded" : "failed");
            if (success) {
                pinRequiresPin = false;
                pinStatusValid = true; 
                lastPinStatusUpdateTime = std::chrono::steady_clock::now();
                updatePinStatusInternal(); 
            }
            if (completionCallback) {
                completionCallback(success);
            }
            return false;
        };
        requests.push_back(request);
    }

    void setFlightMode(bool enableFlightMode, std::function<void(bool success)> completionCallback) {
        auto request = std::make_shared<Request>();
        request->command = "AT+CFUN=" + std::string(enableFlightMode ? "4" : "1");
        request->callback = [completionCallback, enableFlightMode](const std::string& response) -> bool {
            bool success = (response.find("OK") != std::string::npos);
             // printf("[GSM Action] setFlightMode(%s) command %s.\n", enableFlightMode ? "ON" : "OFF", success ? "accepted" : "rejected");
            if (success) {
                flightModeState = enableFlightMode;
                flightModeStateValid = true; 
                lastFlightModeUpdateTime = std::chrono::steady_clock::now();
            }
            if (completionCallback) {
                completionCallback(success); 
            }
            return false;
        };
        requests.push_back(request);
    }

     void setPduMode(bool enablePdu, std::function<void(bool success)> completionCallback) {
        auto request = std::make_shared<Request>();
        request->command = "AT+CMGF=" + std::string(enablePdu ? "0" : "1");
        request->callback = [completionCallback, enablePdu](const std::string& response) -> bool {
            bool success = (response.find("OK") != std::string::npos);
            // printf("[GSM Action] setPduMode(%s) %s.\n", enablePdu ? "PDU" : "Text", success ? "succeeded" : "failed");
             if (success) {
                pduModeEnabled = enablePdu;
                pduModeStateValid = true;
                lastPduModeUpdateTime = std::chrono::steady_clock::now();
             }
            if (completionCallback) {
                completionCallback(success);
            }
            return false;
        };
        requests.push_back(request);
    }

    void sendMessagePDU(const std::string& pdu, int length, std::function<void(bool success, int messageRef)> completionCallback) {
        auto request1 = std::make_shared<Request>();
        request1->command = "AT+CMGS=" + std::to_string(length);
        auto request2 = std::make_shared<Request>();
        request2->command = pdu + "\x1A";

        request1->callback = [completionCallback](const std::string& response) -> bool {
            std::string trimmed_response = response;
            size_t last_char_pos = trimmed_response.find_last_not_of(" \t\r\n");
            if (last_char_pos != std::string::npos && trimmed_response[last_char_pos] == '>') {
                // printf("[GSM Action] PDU prompt received.\n");
                return true; 
            } else {
                // printf("[GSM Action] Failed to get PDU prompt '>'. Response block:\n%s---\n", response.c_str());
                if (completionCallback) completionCallback(false, -1);
                return false; 
            }
        };

        request2->callback = [completionCallback](const std::string& response) -> bool {
            int messageRef = -1;
            bool success = false;
            auto pos = response.find("+CMGS:");
            if (pos != std::string::npos) {
                std::string mr_str = response.substr(pos + 6);
                 size_t first_digit = mr_str.find_first_of("0123456789");
                 size_t last_digit = mr_str.find_first_not_of("0123456789", first_digit);
                 if (first_digit != std::string::npos) {
                     mr_str = mr_str.substr(first_digit, last_digit == std::string::npos ? std::string::npos : last_digit - first_digit);
                     try { messageRef = std::stoi(mr_str); } catch (...) { /* ignore */ }
                 }
                if (response.find("OK") != std::string::npos) {
                    success = true;
                } else {
                     // printf("[GSM Action] +CMGS found but no OK received.\n");
                }
            } else if (response.find("OK") != std::string::npos) {
                 // printf("[GSM Action] OK received without +CMGS reference.\n");
                 success = false; 
            } else if (response.find("ERROR") != std::string::npos) {
                 success = false;
            }
            // printf("[GSM Action] sendMessagePDU %s (MR: %d).\n", success ? "succeeded" : "failed", messageRef);
            if (completionCallback) {
                completionCallback(success, messageRef);
            }
            return false; 
        };
        request1->next = request2; 
        requests.push_back(request1); 
    }

    static std::string byteToHex(unsigned char byte) {
        std::stringstream ss;
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        return ss.str();
    }

    static std::string encodePhoneNumber(const std::string& number) {
        std::string encoded;
        int len = number.length();
        for (int i = 0; i < len; i += 2) {
            if (i + 1 < len) {
                encoded += number[i + 1];
                encoded += number[i];
            } else {
                encoded += 'F';
                encoded += number[i];
            }
        }
        return encoded;
    }
    
    static std::string pack7Bit(const std::string& message, int& septetCount) {
        std::string packedDataHex;
        std::vector<unsigned char> packedOctets;
        unsigned char currentOctet = 0;
        int bitsInCurrentOctet = 0;
        septetCount = 0;

        for (char c_char : message) {
            unsigned char septet;
            // GSM 03.38 Default Alphabet mapping (simplified, consistent with gsm7SeptetToChar)
            // This should be the inverse of gsm7SeptetToChar for characters it handles.
            // For simplicity, using a direct approach here.
            if (c_char == '@') septet = 0x00;
            else if (c_char == '\xA3') septet = 0x01; // £
            else if (c_char == '$') septet = 0x02;
            else if (c_char == '\xA5') septet = 0x03; // ¥
            else if (c_char == '\xE8') septet = 0x04; // è
            else if (c_char == '\xE9') septet = 0x05; // é
            else if (c_char == '\xF9') septet = 0x06; // ù
            else if (c_char == '\xEC') septet = 0x07; // ì
            else if (c_char == '\xF2') septet = 0x08; // ò
            else if (c_char == '\xC7') septet = 0x09; // Ç
            else if (c_char == '\n') septet = 0x0A;
            else if (c_char == '\xD8') septet = 0x0B; // Ø
            else if (c_char == '\xF8') septet = 0x0C; // ø
            else if (c_char == '\r') septet = 0x0D;
            else if (c_char == '\xC5') septet = 0x0E; // Å
            else if (c_char == '\xE5') septet = 0x0F; // å
            // Delta, Phi, Gamma, Lambda, Omega, Pi, Psi, Sigma, Theta, Xi (0x10-0x1A)
            // For simplicity, map common ASCII to their direct values if they exist in GSM7
            else if (c_char == '_') septet = 0x11;
            // ESC (0x1B) - not handled for basic packing
            else if (c_char == '\xC6') septet = 0x1C; // Æ
            else if (c_char == '\xE6') septet = 0x1D; // æ
            else if (c_char == '\xDF') septet = 0x1E; // ß
            else if (c_char == '\xC9') septet = 0x1F; // É
            else if (c_char == ' ') septet = 0x20;
            else if (c_char >= 'A' && c_char <= 'Z') septet = static_cast<unsigned char>(c_char);
            else if (c_char >= 'a' && c_char <= 'z') septet = static_cast<unsigned char>(c_char);
            else if (c_char >= '0' && c_char <= '9') septet = static_cast<unsigned char>(c_char);
            else { // Fallback for other common ASCII chars or map to '?'
                switch (c_char) {
                    case '!': septet = 0x21; break; case '"': septet = 0x22; break;
                    case '#': septet = 0x23; break; case '%': septet = 0x25; break;
                    case '&': septet = 0x26; break; case '\'': septet = 0x27; break;
                    case '(': septet = 0x28; break; case ')': septet = 0x29; break;
                    case '*': septet = 0x2A; break; case '+': septet = 0x2B; break;
                    case ',': septet = 0x2C; break; case '-': septet = 0x2D; break;
                    case '.': septet = 0x2E; break; case '/': septet = 0x2F; break;
                    case ':': septet = 0x3A; break; case ';': septet = 0x3B; break;
                    case '<': septet = 0x3C; break; case '=': septet = 0x3D; break;
                    case '>': septet = 0x3E; break; case '?': septet = 0x3F; break;
                    default: septet = 0x3F; break; // Unknown char to '?'
                }
            }
            septet &= 0x7F;
            septetCount++;

            // Shift septet into currentOctet
            // The LSBs of septet go into MSBs of currentOctet's remaining space
            currentOctet |= (septet << bitsInCurrentOctet);
            int bits_from_septet_for_this_octet = std::min(7, 8 - bitsInCurrentOctet);
            
            bitsInCurrentOctet += 7;

            if (bitsInCurrentOctet >= 8) {
                packedOctets.push_back(currentOctet);
                bitsInCurrentOctet -= 8;
                currentOctet = (septet >> (7 - bitsInCurrentOctet)); // Carry over remaining bits
            }
        }

        // If there are remaining bits in currentOctet after all septets processed
        if (bitsInCurrentOctet > 0 && septetCount > 0) { // septetCount check for empty message
            packedOctets.push_back(currentOctet);
        }
        
        for(unsigned char oct : packedOctets) {
            packedDataHex += byteToHex(oct);
        }
        return packedDataHex;
    }


    std::pair<std::string, int> encodePduSubmit(const std::string& recipientNumber, const std::string& message) {
        std::string pdu = "";
        int cmgsLength = 0;
        pdu += "00";
        pdu += "01"; cmgsLength++;
        pdu += "00"; cmgsLength++;

        std::string cleanNumber = recipientNumber;
        bool isInternational = false;
        if (!cleanNumber.empty() && cleanNumber[0] == '+') {
            isInternational = true;
            cleanNumber = cleanNumber.substr(1);
        }
        cleanNumber.erase(std::remove_if(cleanNumber.begin(), cleanNumber.end(),
                                         [](char c){ return !std::isdigit(c); }),
                          cleanNumber.end());

        if (cleanNumber.empty()) {
            // printf("[PDU Encode] Error: Recipient number is empty or invalid.\n");
            return {"", -1}; 
        }
        pdu += byteToHex(static_cast<unsigned char>(cleanNumber.length())); cmgsLength++;
        pdu += (isInternational ? "91" : "81"); cmgsLength++;
        std::string encodedNumber = encodePhoneNumber(cleanNumber);
        pdu += encodedNumber;
        cmgsLength += encodedNumber.length() / 2; 
        pdu += "00"; cmgsLength++;
        pdu += "00"; cmgsLength++;

        int septetCount = 0; 
        std::string packedUserData = pack7Bit(message, septetCount);
        if (septetCount > 160) {
             // printf("[PDU Encode] Warning: Message length (%d septets) exceeds 160.\n", septetCount);
        }
        pdu += byteToHex(static_cast<unsigned char>(septetCount)); cmgsLength++;
        pdu += packedUserData;
        cmgsLength += static_cast<int>(std::ceil(septetCount * 7.0 / 8.0));

        std::transform(pdu.begin(), pdu.end(), pdu.begin(), ::toupper);
        return {pdu, cmgsLength};
    }


    void sendMySms(const std::string& recipient, const std::string& text) {
        std::pair<std::string, int> pduData = Gsm::encodePduSubmit(recipient, text);
        if (pduData.second == -1) {
            // printf("[SMS Example] Failed to encode PDU for SMS.\n");
            return;
        }
        std::string pduString = pduData.first;
        int cmgsLength = pduData.second;
        // printf("[SMS Example] PDU to send: %s\n", pduString.c_str());
        // printf("[SMS Example] Length for AT+CMGS: %d\n", cmgsLength);

        Gsm::sendMessagePDU(pduString, cmgsLength, [recipient, text](bool success, int messageRef) {
            if (success) {
                // printf("[SMS Example] SMS sent successfully! Message Reference: %d\n", messageRef);

                Conversations::Conversation conv;
                storage::Path convPath(std::string(MESSAGES_LOCATION) + "/" + recipient + ".json");
                if (convPath.exists())
                {
                    Conversations::loadConversation(convPath, conv);
                }
                else
                {
                    conv.number = recipient;
                }
                conv.messages.push_back({text, false, std::to_string(getCurrentTimestamp())}); // false = message de l'user
                Conversations::saveConversation(convPath, conv);
                
            } else {
                // printf("[SMS Example] Failed to send SMS.\n");
            }
        });
    }

    void call(const std::string& number, std::function<void(bool success)> completionCallback) {
        if (currentCallState != CallState::IDLE) {
            // printf("[GSM Action] Cannot call, state is not IDLE (%d).\n", static_cast<int>(currentCallState));
            if (completionCallback) completionCallback(false);
            return;
        }
        auto request = std::make_shared<Request>();
        request->command = "ATD" + number + ";"; 
        request->callback = [completionCallback, number](const std::string& response) -> bool {
            bool success = (response.find("OK") != std::string::npos);
            // printf("[GSM Action] call(%s) command %s.\n", number.c_str(), success ? "accepted" : "rejected");
            if (success) {
                currentCallState = CallState::DIALING; 
            }
            if (completionCallback) {
                completionCallback(success); 
            }
            return false;
        };
        requests.push_back(request);
    }

    void acceptCall(std::function<void(bool success)> completionCallback) {
         if (currentCallState != CallState::RINGING) {
             // printf("[GSM Action] Cannot accept call, state is not RINGING (%d).\n", static_cast<int>(currentCallState));
            if (completionCallback) completionCallback(false);
            return;
        }
        auto request = std::make_shared<Request>();
        request->command = "ATA";
        request->callback = [completionCallback](const std::string& response) -> bool {
            bool success = (response.find("OK") != std::string::npos);
            // printf("[GSM Action] acceptCall command %s.\n", success ? "accepted" : "rejected");
            if (success) {
                currentCallState = CallState::ACTIVE; 
            } else {
                 // printf("[GSM Action] ATA command failed. Call state might be inconsistent.\n");
            }
            if (completionCallback) {
                completionCallback(success); 
            }
            return false;
        };
        requests.push_back(request);
    }

    void rejectCall(std::function<void(bool success)> completionCallback) {
         if (currentCallState == CallState::IDLE || currentCallState == CallState::UNKNOWN) {
             // printf("[GSM Action] Cannot reject/hangup, no call active/ringing/dialing (%d).\n", static_cast<int>(currentCallState));
            if (completionCallback) completionCallback(false);
            return;
        }
        auto request = std::make_shared<Request>();
        request->command = "AT+CHUP";
        request->callback = [completionCallback](const std::string& response) -> bool {
            bool success = (response.find("OK") != std::string::npos);
             // printf("[GSM Action] rejectCall/hangup command %s.\n", success ? "accepted" : "rejected");
            if (success) {
                currentCallState = CallState::IDLE;
                lastIncomingCallNumber = "";
            } else {
                 // printf("[GSM Action] AT+CHUP command failed. Call state might be inconsistent.\n");
            }
            if (completionCallback) {
                completionCallback(success); 
            }
            return false;
        };
        requests.push_back(request);
    }

    void httpGet(const std::string& url, HttpGetCallbacks callbacks) {
        if (currentHttpState != HttpState::IDLE) {
            // printf("[GSM HTTP] Error: Cannot start GET. State is not IDLE.\n");
            if (callbacks.on_complete) {
                // TODO: Queue the request
                callbacks.on_complete();
            }
            return;
        }

        // printf("[GSM HTTP] Transaction started for URL: %s\n", url.c_str());

        currentHttpState = HttpState::INITIALIZING;
        currentHttpCallbacks = std::move(callbacks);
        httpBytesTotal = 0;
        httpBytesRead = 0;

        auto initReq = std::make_shared<Request>();
        auto setCidReq = std::make_shared<Request>();
        auto setUrlReq = std::make_shared<Request>();
        auto actionReq = std::make_shared<Request>();

        // 1. INIT
        initReq->command = "AT+HTTPINIT";
        initReq->callback = [](const std::string& response) -> bool {
            // printf("[GSM HTTP] INIT response: %s\n", response.c_str());
            if (response.find("OK") == std::string::npos) {
                _completeHttpRequest(HttpResult::INIT_FAILED);
                return false;
            }
            return true;
        };

        // 2. SET CID
        /*setCidReq->command = "AT+HTTPPARA=\"CID\",1";
        setCidReq->callback = [](const std::string& response) -> bool {
            if (response.find("OK") == std::string::npos) {
                _completeHttpRequest(HttpResult::MODULE_ERROR);
                return false;
            }
            return true;
        };*/

        // 3. SET URL
        setUrlReq->command = "AT+HTTPPARA=\"URL\",\"" + url + "\"";
        setUrlReq->callback = [](const std::string& response) -> bool {
            if (response.find("OK") == std::string::npos) {
                _completeHttpRequest(HttpResult::MODULE_ERROR);
                return false;
            }
            return true;
        };

        // 4. ACTION
        actionReq->command = "AT+HTTPACTION=0";
        actionReq->callback = [](const std::string& response) -> bool {
            if (response.find("OK") == std::string::npos) {
                _completeHttpRequest(HttpResult::MODULE_ERROR);
                return false;
            }
            currentHttpState = HttpState::ACTION_IN_PROGRESS;
            return false;
        };

        initReq->next = setUrlReq;
        //setCidReq->next = setUrlReq;
        setUrlReq->next = actionReq;
        requests.push_back(initReq);
    }

    static bool onHttpReadBlock(const std::string& response) {
        if(response.find("OK") == std::string::npos) {
            // printf("[GSM HTTP] Read block failed, response: %s\n", response.c_str());
            _completeHttpRequest(HttpResult::READ_ERROR);
            return false; // This command is done, don't chain to a `next`.
        }

        return false;

        // The response contains: +HTTPREAD: <len>\r\n<binary data>\r\nOK\r\n
        size_t header_start = response.find("+HTTPREAD:");
        if (header_start == std::string::npos) {
            _completeHttpRequest(HttpResult::READ_ERROR);
            return false;
        }

        size_t header_end = response.find("\r\n", header_start);
        if (header_end == std::string::npos) {
            _completeHttpRequest(HttpResult::READ_ERROR);
            return false;
        }

        // Extract the length of this specific chunk
        int chunk_len = 0;
        sscanf(response.c_str() + header_start, "+HTTPREAD: %d", &chunk_len);

        if (chunk_len > 0) {
            size_t data_start = header_end + 2; // Skip the \r\n
            
            if (data_start + chunk_len > response.length()) {
                _completeHttpRequest(HttpResult::READ_ERROR);
                return false;
            }

            // Create a span of the data chunk and call the user's callback
            if (currentHttpCallbacks.on_data) {
                std::string_view data_chunk(response.data() + data_start, chunk_len);
                // printf("[GSM HTTP] Read %d bytes of data.\n", chunk_len);
                currentHttpCallbacks.on_data(data_chunk);
            }
            httpBytesRead += chunk_len;
        }

        // After processing the block, decide the next step.
        if (httpBytesRead >= httpBytesTotal) {
            _completeHttpRequest(HttpResult::OK);
        } else {
            _queueNextHttpRead();
        }

        return false; // This command is done, don't chain to a `next`.
    }

    static void _queueNextHttpRead() {
        auto readReq = std::make_shared<Request>();
        readReq->command = "AT+HTTPREAD=1024";
        readReq->callback = onHttpReadBlock;
        requests.push_back(readReq);
    }

    // Helper to centralize cleanup and callback invocation
    static void _completeHttpRequest(HttpResult result) {
        // printf("[GSM HTTP] Completing transaction with result: %d\n", static_cast<int>(result));

        if (currentHttpCallbacks.on_complete) {
            currentHttpCallbacks.on_complete();
        }

        currentHttpCallbacks = {};
        currentHttpState = HttpState::TERMINATING;

        auto termReq = std::make_shared<Request>();
        termReq->command = "AT+HTTPTERM";
        termReq->callback = [](const std::string&) -> bool {
            // printf("[GSM HTTP] Session terminated.\n");
            currentHttpState = HttpState::IDLE;
            return false;
        };
        requests.insert(requests.begin(), termReq);
    }

    // --- Public Refresh Functions ---
    void refreshNetworkQuality() { updateNetworkQualityInternal(); }
    void refreshConnectionStatus() { updateGprsAttachmentStatusInternal(); }
    void refreshFlightModeStatus() { updateFlightModeStatusInternal(); }
    void refreshVoltage() { updateVoltageInternal(); }
    void refreshPinStatus() { updatePinStatusInternal(); }
    void refreshPduModeStatus() { updatePduModeStatusInternal(); }


    // --- Main Processing Loop (run) ---
    void run()
    {
        #ifdef ESP_PLATFORM
        enum class SerialRunState { NO_COMMAND, COMMAND_RUNNING, SENDING_PDU_DATA };
        static SerialRunState state = SerialRunState::NO_COMMAND;
        static std::chrono::steady_clock::time_point lastCommandTime;
        const std::chrono::milliseconds commandTimeoutDuration(15000); 
        const std::chrono::milliseconds pduTimeoutDuration(30000); 

        static std::string currentResponseBlock = "";
        currentResponseBlock.reserve(2048);
        static std::string lineBuffer;
        lineBuffer.reserve(2048);

        if (state == SerialRunState::NO_COMMAND && !requests.empty())
        {
            currentRequest = requests.front();
            requests.erase(requests.begin()); 

            // printf("[GSM TX]: %s\n", currentRequest->command.c_str());
            currentResponseBlock = ""; 

            if (currentRequest->command.find('\x1A') != std::string::npos) {
                 gsm.print(currentRequest->command.c_str()); 
                 state = SerialRunState::SENDING_PDU_DATA;
                 lastCommandTime = std::chrono::steady_clock::now();
            } else {
                gsm.print(currentRequest->command.c_str());
                gsm.print('\r'); 
                state = SerialRunState::COMMAND_RUNNING;
                lastCommandTime = std::chrono::steady_clock::now();
            }
        }

        std::string incomingData = "";
        #ifdef ESP_PLATFORM
        while (gsm.available()) {
            char c = gsm.read();
            incomingData += c;
        }
        #endif

        if (!incomingData.empty()) {
            lineBuffer += incomingData;
        }

        size_t lineEndPos;
        while ((lineEndPos = lineBuffer.find('\n')) != std::string::npos)
        {
            std::string line = lineBuffer.substr(0, lineEndPos);
            lineBuffer.erase(0, lineEndPos + 1); 

            if (!line.empty() && line.back() == '\r') line.pop_back();
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);

            if (line.empty()) continue; 

            if ((state == SerialRunState::COMMAND_RUNNING) && currentRequest &&
                 line == currentRequest->command)
            {
                 // printf("[GSM] Echo ignored: %s\n", line.c_str());
                 continue; 
            }

            bool potentialURC = isURC(line);
            bool isFinalReply = isEndIdentifier(line);

            if (potentialURC && (state == SerialRunState::NO_COMMAND || !isFinalReply)) {
                processURC(line);
                continue;
            }

            if(line.find("+HTTPREAD:") != std::string::npos)
            {
                // printf("[GSM HTTP] Reading HTTP data block: %s\n", line.c_str());
                size_t datasize = 0;
                if (sscanf(line.c_str(), "+HTTPREAD: %zu", &datasize) == 1) {
                    // printf("[GSM HTTP] Expected data size: %zu bytes\n", datasize);
                } else {
                    // printf("[GSM HTTP] Failed to parse data size from line: %s\n", line.c_str());
                }

                if (datasize > 0) {
                    std::string data;
                    data.reserve(datasize);

                    // Read buffered data first
                    if (!lineBuffer.empty()) {
                        size_t to_copy = std::min(datasize, lineBuffer.size());
                        data.append(lineBuffer.substr(0, to_copy));
                        lineBuffer.erase(0, to_copy);
                        datasize -= to_copy;
                    }

                    // Continue reading from gsm until size is correct
                    while (datasize > 0 && gsm.available()) {
                        char c = gsm.read();
                        data.push_back(c);
                        datasize--;
                    }

                    if (data.size() == data.capacity()) {
                        // printf("[GSM HTTP] Complete data read successfully: %d bytes\n", data.size());
                        std::string_view dataView(data.data(), data.size());
                        if (currentHttpCallbacks.on_data)
                            currentHttpCallbacks.on_data(dataView);
                        httpBytesRead += data.size();

                        if (httpBytesRead >= httpBytesTotal) {
                            _completeHttpRequest(HttpResult::OK);
                        } else {
                            _queueNextHttpRead(); 
                        }
                    } else {
                        // printf("[GSM HTTP] Incomplete data read. Expected: %zu, Read: %zu\n", data.capacity(), data.size());
                        _completeHttpRequest(HttpResult::READ_ERROR);
                    }
                }
            }

            if (state == SerialRunState::COMMAND_RUNNING || state == SerialRunState::SENDING_PDU_DATA)
            {
                currentResponseBlock += line + "\n";
                bool isPduPrompt = (line == ">" && state == SerialRunState::COMMAND_RUNNING);

                if (isFinalReply || isPduPrompt)
                {
                    bool executeNext = false; 
                    if (currentRequest && currentRequest->callback) {
                        executeNext = currentRequest->callback(currentResponseBlock);
                    }

                    if (isPduPrompt && executeNext && currentRequest && currentRequest->next) {
                        requests.insert(requests.begin(), currentRequest->next);
                        // printf("[GSM] PDU prompt handled, queuing PDU data send.\n");
                        currentRequest = nullptr;
                        state = SerialRunState::NO_COMMAND;
                        currentResponseBlock = ""; 
                    }
                    else if (isFinalReply) {
                        if (executeNext && currentRequest && currentRequest->next) {
                            requests.insert(requests.begin(), currentRequest->next);
                            // printf("[GSM] Chaining next request.\n");
                        }
                        currentRequest = nullptr;
                        state = SerialRunState::NO_COMMAND; 
                        currentResponseBlock = ""; 
                    }
                    else if (isPduPrompt) {
                         // printf("[GSM] PDU prompt received but no chaining requested or possible.\n");
                         currentRequest = nullptr;
                         state = SerialRunState::NO_COMMAND;
                         currentResponseBlock = "";
                    }
                } 
            } 
            else if (!potentialURC) { 
                 // printf("[GSM] Unexpected data (State: NO_COMMAND): %s\n", line.c_str());
            }
        } 


        if (state != SerialRunState::NO_COMMAND) {
            auto timeout = (state == SerialRunState::SENDING_PDU_DATA) ? pduTimeoutDuration : commandTimeoutDuration;
            if ((std::chrono::steady_clock::now() - lastCommandTime) > timeout) {
                // printf("[GSM] Command Timeout! State: %d, Command: %s\n",
                //       static_cast<int>(state),
                //       currentRequest ? currentRequest->command.c_str() : "N/A");

                if (currentRequest && currentRequest->callback) {
                    currentRequest->callback("TIMEOUT_ERROR"); 
                }
                currentRequest = nullptr;
                currentResponseBlock = "";
                lineBuffer = ""; 
                state = SerialRunState::NO_COMMAND;
            }
        }
        #endif
    }

    void downloadFile(const std::string& url) {
        Gsm::HttpGetCallbacks my_callbacks;
        size_t file_data_size = 0;

        my_callbacks.on_init = [](Gsm::HttpResult result) {
            std::cout << "HTTP GET operation initialized: " << static_cast<int>(result) << std::endl;
        };

        // This lambda now accepts a pointer and a size.
        my_callbacks.on_data = [&](const std::string_view &data) {
            std::cout << "Received data chunk of size: " << data.size() << " bytes." << std::endl;
            /*std::cout << "============== DATA CHUNK ==============" << std::endl;
            std::cout << data << std::endl;
            std::cout << "=========================================" << std::endl;*/
            file_data_size += data.length();
        };

        my_callbacks.on_complete = [&]() {
            std::cout << "Download complete! Total size: " << file_data_size << " bytes." << std::endl;
        };

        // Start the operation (this call doesn't change)
        Gsm::httpGet(url, my_callbacks);
    }


    void loop()
    {
        eventHandlerGsm.setInterval([&]() {
            refreshNetworkQuality();
        }, 5000); 

        eventHandlerGsm.setInterval([&]() {
            // printf("[GSM] Running medium frequency checks.\n");
            refreshConnectionStatus();
            refreshVoltage();
            refreshPinStatus();
        }, 30000); 

        eventHandlerGsm.setInterval([&]() {
            Time::syncNetworkTime();
        }, 5000);


        eventHandlerGsm.setTimeout(new Callback<>([](){ downloadFile("https://www.google.com"); }), 10000); 

        while (true) {
            StandbyMode::buisy_io.lock();
            do
            {
                run(); 
                eventHandlerGsm.update();
                PaxOS_Delay(1);
            } while (requests.size() > 0 || currentRequest != nullptr);
            
            StandbyMode::buisy_io.unlock();
            PaxOS_Delay(4);
        }
    }

} // namespace Gsm