#ifndef URLRESPONSE_HPP
#define URLRESPONSE_HPP

#include <cstdint>

namespace network {
    struct URLResponse {
        URLResponse(uint16_t statusCode, uint64_t responseBodySize) : statusCode(statusCode), responseBodySize(responseBodySize) {}

        uint16_t statusCode;

        uint64_t responseBodySize;

        uint64_t readPosition = 0;
    };
}

#endif // URLRESPONSE_HPP