#ifndef URLRESPONSE_HPP
#define URLRESPONSE_HPP

#include <cstdint>

namespace network {
    struct URLResponse {
        URLResponse(uint16_t statusCode, uint64_t responseBodySize) : statusCode(statusCode), responseBodySize(responseBodySize) {}

        /*
        * The status code of the response

        For codes < 600 see online documentation, for codes >= 700 see the documentation below:
        - 700: The URL is not valid
        - 701: No internet connection
        - 702: The request has been cancelled by the user
        - 703: The cURL handle is null (only when WiFi is used)
        - 704: The HTTP method is not supported
        - 705: Timout when reading a chunk of the response data (GSM only)
        - 706: Failed to send GET request (GSM only)
        - 707: Failed to send POST request (GSM only)
        - 708: The request has timed out (GSM only)
        - 709: The request is cancelled or finished (GSM only) (should happen rarely)
        */
        uint16_t statusCode;

        uint64_t responseBodySize;

        uint64_t readPosition = 0;
    };
}

#endif // URLRESPONSE_HPP