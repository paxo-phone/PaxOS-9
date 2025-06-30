#ifndef PDU_DECODER_HPP
#define PDU_DECODER_HPP

/*
    PDU decoder:
    Pdu is a format used in the encodage of SMS and MMS. Decoding it is
   necessary to get the sender, the message and the URL of any SMS or MMS.
*/

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
