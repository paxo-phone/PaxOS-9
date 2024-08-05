#ifndef CONVERSATION_HPP
#define CONVERSATION_HPP

#include <vector>
#include <string>
#include <json.hpp>

#include <path.hpp>

#define MESSAGES_LOCATION "apps/messages/data"
#define MESSAGES_NOTIF_LOCATION "apps/messages/unread.txt"
#define MAX_MESSAGES 40

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

    void loadConversation(const storage::Path &filePath, Conversation &conv);
    void saveConversation(const storage::Path &filePath, const Conversation &conv);
}

#endif // CONVERSATION_HPP
