#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <json.hpp>
#include <string>
#include <vector>

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
} // namespace Message

#endif
