#ifndef URL_HPP
#define URL_HPP

#include <string>

namespace network
{
    struct URL 
    {
        URL(const std::string& string);

        const std::string absoluteString;

        bool isValid() const;
    };
}

#endif // URL_HPP