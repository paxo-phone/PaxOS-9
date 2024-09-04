#ifndef PDU_DECODER_HPP
#define PDU_DECODER_HPP

#include <string>

enum PDU_type
{
    SMS,
    MMS,
    UNKNOWN
};

struct PDU
{
    std::string sender;
    std::string message;
    std::string url;
    PDU_type type;
};

PDU decodePDU(std::string pdu);

#endif