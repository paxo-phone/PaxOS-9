#include "URL.hpp"

#include <regex>


namespace network
{
    URL::URL(const std::string& string) : absoluteString(string) {}

    bool URL::isValid() const
    {
        return std::regex_match(absoluteString, std::regex("([a-zA-Z]+)://([^/ :]+)(:([0-9]+))?(/([^ #?]+)?([^ #]+)?)?"));
    }
}