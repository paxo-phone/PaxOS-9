#include "conversation.hpp"
#include <filestream.hpp>
#include <path.hpp>
#include <iostream>
#include <cerrno>
#include <cstring>

namespace Conversations
{
    void loadConversation(const storage::Path &filePath, Conversation &conv)
    {
        //std::cout << "Loading conversation from: " << filePath.str() << std::endl;

        if (!filePath.exists())
        {
            std::cerr << "File does not exist: " << filePath.str() << std::endl;
            saveConversation(filePath, conv);
            return;
        }

        storage::FileStream file;
        file.open(filePath.str(), storage::Mode::READ);
        if (!file.isopen())
        {
            std::cerr << "Failed to open file: " << filePath.str() << " Error: " << strerror(errno) << std::endl;
            return;
        }

        std::string fileContent = file.read();
        file.close();
        //std::cout << "File content: " << fileContent << std::endl;

        if (fileContent.empty())
        {
            //std::cerr << "File is empty: " << filePath.str() << std::endl;
            return;
        }

        try
        {
            auto json = nlohmann::json::parse(fileContent);

            conv.number = json.at("number").get<std::string>();
            conv.messages.clear();
            for (const auto &messageItem : json.at("messages"))
            {
                Message msg;
                msg.message = messageItem.at("message").get<std::string>();
                msg.who = messageItem.at("who").get<bool>();
                msg.date = messageItem.at("date").get<std::string>();
                conv.messages.push_back(msg);
            }
        }
        catch (const nlohmann::json::exception &e)
        {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        }
    }

    void saveConversation(const storage::Path &filePath, const Conversation &conv)
    {
        //std::cout << "Saving conversation to: " << filePath.str() << std::endl;

        storage::Path parentPath = filePath / "../";

        if (!parentPath.exists())
        {
            parentPath.newdir();
        }

        nlohmann::json json;
        json["number"] = conv.number;

        for (int i = ((conv.messages.size() < MAX_MESSAGES)?(0):(conv.messages.size() - MAX_MESSAGES)); i < conv.messages.size(); i++)
        {
            auto& msg = conv.messages[i];

            json["messages"].push_back({{"message", msg.message},
                                        {"who", msg.who},
                                        {"date", msg.date}});
        }

        storage::Path path(filePath);
        storage::FileStream file;
        file.open(path.str(), storage::Mode::WRITE);
        if (!file.isopen())
        {
            std::cerr << "Failed to open file for writing: " << filePath.str() << " Error: " << strerror(errno) << std::endl;
            return;
        }

        std::string jsonString = json.dump(4);
        //std::cout << "Writing JSON: " << jsonString << std::endl;
        file.write(jsonString);
        file.close();
    }
}
