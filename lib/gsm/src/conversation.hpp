#ifndef CONVERSATION_HPP
#define CONVERSATION_HPP

#include <vector>
#include <string>
#include <json.hpp>

#define MESSAGES_LOCATION "apps/messages/data"

namespace Conversations
{
    struct Message
    {
        std::string message;
        bool who; // false = myself, true = the other
        std::string date;
    };

    struct Conversation
    {
        std::string number;
        std::vector<Message> messages;
    };

    void loadConversation(const std::string &filePath, Conversation &conv);
    void saveConversation(const std::string &filePath, const Conversation &conv);
}

#endif // CONVERSATION_HPP
