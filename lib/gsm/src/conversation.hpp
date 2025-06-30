#ifndef CONVERSATION_HPP
#define CONVERSATION_HPP

#include <json.hpp>
#include <path.hpp>
#include <string>
#include <vector>

#define MESSAGES_LOCATION "apps/messages/data"
#define MESSAGES_IMAGES_LOCATION "apps/messages/images"
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

    /**
     * @brief Load a conversation from a file
     * @param filePath The file path containing the conversation to load
     * @param conv The conversation to load into
     */
    void loadConversation(const storage::Path& filePath, Conversation& conv);
    /**
     * @brief Save a conversation to a file
     * @param filePath The file path to save the conversation to
     * @param conv The conversation to save
     */
    void saveConversation(const storage::Path& filePath, const Conversation& conv);
} // namespace Conversations

#endif // CONVERSATION_HPP
