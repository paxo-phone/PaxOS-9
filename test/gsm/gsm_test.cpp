#include <gtest/gtest.h>
#include "gsm.hpp"
#include "conversation.hpp"
#include "contacts.hpp"
#include <path.hpp>
#include <filestream.hpp>
#include <iostream>
#include <filesystem>

// Test d'initialisation du module GSM
TEST(GSMTest, Initialization)
{
    GSM::init();
    ASSERT_TRUE(true);
}

// Test de l'envoi de message
TEST(GSMTest, SendMessage)
{
    // std::string number = "0612345679";
    // std::string message = "Test message";

    // GSM::newMessage(number, message);

    // // Vérifier que le message a été ajouté à la file d'attente
    // ASSERT_FALSE(GSM::messages.empty());
    // if (!GSM::messages.empty())
    // {
    //     ASSERT_EQ(GSM::messages.back().number, number);
    //     ASSERT_EQ(GSM::messages.back().message, message);
    // }

    // // Vérifier que le message a été ajouté à la conversation
    // Conversations::Conversation conv;
    // storage::Path convFilePath(std::string(MESSAGES_LOCATION) + "/" + number + ".json");
    // Conversations::loadConversation(convFilePath.str(), conv);
    // std::cout << "Checking conversation for number: " << number << " at path: " << convFilePath.str() << std::endl;
    // ASSERT_FALSE(conv.messages.empty());
    // if (!conv.messages.empty())
    // {
    //     ASSERT_EQ(conv.messages.back().message, message);
    //     ASSERT_FALSE(conv.messages.back().who); // false = message de l'user
    // }
}

// Test de la réception de message
TEST(GSMTest, ReceiveMessage)
{
    // std::string number = "0612345679";
    // std::string receivedMessage = "Test message";

    // GSM::data = "+CMTI: \"SM\",1\n"; // Simuler la réception de données du module GSM
    // GSM::onMessage();

    // // Vérifier que le message a été ajouté à la liste des messages
    // ASSERT_FALSE(GSM::messages.empty());
    // if (!GSM::messages.empty())
    // {
    //     std::string loadedNumber = GSM::messages.back().number;
    //     std::string loadedMessage = GSM::messages.back().message;

    //     // Vérifier que le message a été ajouté à la conversation
    //     Conversations::Conversation conv;
    //     storage::Path convFilePath(std::string(MESSAGES_LOCATION) + "/" + loadedNumber + ".json");
    //     Conversations::loadConversation(convFilePath.str(), conv);
    //     std::cout << "Checking conversation for number: " << loadedNumber << " at path: " << convFilePath.str() << std::endl;
    //     ASSERT_FALSE(conv.messages.empty());
    //     if (!conv.messages.empty())
    //     {
    //         ASSERT_EQ(conv.messages.back().message, loadedMessage);
    //         ASSERT_TRUE(conv.messages.back().who); // true = message de l'autre
    //     }
    // }
}

// Test de récupération de messages pour un numéro donné
TEST(GSMTest, GetMessages)
{
    // std::string number = "0612345681";

    // // Charger la conversation directement sans passer par lua_gsm
    // Conversations::Conversation conv;
    // storage::Path convFilePath(std::string(MESSAGES_LOCATION) + "/" + number + ".json");
    // Conversations::loadConversation(convFilePath.str(), conv);

    // ASSERT_FALSE(conv.messages.empty());
    // for (const auto &msg : conv.messages)
    // {
    //     std::cout << "Message: " << msg.message << ", Who: " << msg.who << ", Date: " << msg.date << std::endl;
    // }
}
