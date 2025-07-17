#include "conversation.hpp"
#include "path.hpp"

#include <filestream.hpp>
#include <gtest/gtest.h>
#include <iostream>

TEST(ConversationTest, SaveLoadConversation)
{
    // Chemin du répertoire des messages
    storage::Path messagesDir(MESSAGES_LOCATION);
    std::cout << "Checking if directory exists: " << messagesDir.str() << std::endl;
    if (!messagesDir.exists())
    {
        std::cout << "Directory does not exist. Creating..." << std::endl;
        std::string command = "mkdir -p " + messagesDir.str();
        system(command.c_str());
    }

    // Nom du fichier de conversation
    std::string number = "0612345678";
    storage::Path convFilePath = messagesDir / (number + ".json");
    std::cout << "Conversation file path: " << convFilePath.str() << std::endl;

    // Création d'une nouvelle conversation
    Conversations::Conversation conv;
    conv.number = number;
    conv.messages = {
        {"Hello", false, "2024-01-01 10:00:00"},
        {"Hi there!", true, "2024-01-01 10:05:00"}
    };

    // Sauvegarde de la conversation
    storage::FileStream writeStream(convFilePath.str(), storage::Mode::WRITE);
    nlohmann::json json;
    json["number"] = conv.number;

    for (const auto& msg : conv.messages)
    {
        json["messages"].push_back(
            {{"message", msg.message}, {"who", msg.who}, {"date", msg.date}}
        );
    }
    writeStream.write(json.dump(4));
    writeStream.close();

    // Chargement de la conversation sauvegardée
    storage::FileStream readStream(convFilePath.str(), storage::Mode::READ);
    std::string fileContent = readStream.read();
    readStream.close();

    nlohmann::json loadedJson = nlohmann::json::parse(fileContent);
    Conversations::Conversation loadedConv;
    loadedConv.number = loadedJson["number"].get<std::string>();

    for (const auto& item : loadedJson["messages"])
    {
        Conversations::Message msg{
            item["message"].get<std::string>(),
            item["who"].get<bool>(),
            item["date"].get<std::string>()
        };
        loadedConv.messages.push_back(msg);
    }

    // Vérification de l'intégrité des données
    ASSERT_EQ(conv.number, loadedConv.number);
    ASSERT_EQ(conv.messages.size(), loadedConv.messages.size());

    for (size_t i = 0; i < conv.messages.size(); ++i)
    {
        ASSERT_EQ(conv.messages[i].message, loadedConv.messages[i].message);
        ASSERT_EQ(conv.messages[i].who, loadedConv.messages[i].who);
        ASSERT_EQ(conv.messages[i].date, loadedConv.messages[i].date);
    }
}
