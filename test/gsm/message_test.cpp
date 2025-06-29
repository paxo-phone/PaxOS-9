#include "message.hpp"

#include <gtest/gtest.h>

// Test de sauvegarde et de chargement des messages
TEST(MessageTest, SaveLoadMessages)
{
    std::vector<Message::Message> messages = {
        {"0612345678", "Hello", "2024-01-01 10:00:00"},
        {"0612345679", "World", "2024-01-01 10:05:00"}
    };
    Message::saveMessages("/tmp/messages.json", messages);

    std::vector<Message::Message> loadedMessages;
    Message::loadMessages("/tmp/messages.json", loadedMessages);

    ASSERT_EQ(messages.size(), loadedMessages.size());
    for (size_t i = 0; i < messages.size(); ++i)
    {
        ASSERT_EQ(messages[i].number, loadedMessages[i].number);
        ASSERT_EQ(messages[i].message, loadedMessages[i].message);
        ASSERT_EQ(messages[i].date, loadedMessages[i].date);
    }
}
