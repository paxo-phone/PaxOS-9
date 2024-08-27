#ifndef URLREQUEST_HPP
#define URLREQUEST_HPP

#include <map>
#include <string>
#include "URL.hpp"

namespace network {
    struct URLRequest {
        enum HTTPMethod {
            GET,
            POST,
            PUT,
            DELETE
        };

        URLRequest(URL url) : url(url), method(GET), timeoutInterval(60000) {}

        HTTPMethod method;

        URL url;

        std::string httpBody;

        std::map<std::string, std::string> httpHeaderFields;

        uint64_t timeoutInterval; // in ms
    };
}

#endif // URLREQUEST_HPP