#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <vector>
#include <string>
#include <json.hpp>

namespace Message
{
    struct Message
    {
        std::string number;
        std::string message;
        std::string date;
    };

    void loadMessages(const std::string& filePath, std::vector<Message>& messages);
    void saveMessages(const std::string& filePath, const std::vector<Message>& messages);
}

#endif
