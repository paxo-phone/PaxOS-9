#include "conversation.hpp"
#include <filestream.hpp>
#include <path.hpp>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <filesystem>

namespace Conversations
{
    void loadConversation(const std::string &filePath, Conversation &conv)
    {
        std::cout << "Loading conversation from: " << filePath << std::endl;
        storage::Path path(filePath);

        if (!path.exists())
        {
            std::cerr << "File does not exist: " << filePath << std::endl;
            return;
        }

        storage::FileStream file;
        file.open(path.str(), storage::Mode::READ);
        
        if (!file.isopen())
        {
            std::cerr << "Failed to open file: " << filePath << " Error: " << strerror(errno) << std::endl;
            return;
        }

        std::string fileContent = file.read();
        file.close();
        std::cout << "File content: " << fileContent << std::endl;

        if (fileContent.empty())
        {
            std::cerr << "File is empty: " << filePath << std::endl;
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

    void saveConversation(const std::string &filePath, const Conversation &conv)
    {
        std::cout << "Saving conversation to: " << filePath << std::endl;

        // Create directory if it doesn't exist
        size_t lastSlashPos = filePath.find_last_of('/');

        if (lastSlashPos != std::string::npos)
        {
            std::string parentPath = filePath.substr(0, lastSlashPos);
            std::filesystem::create_directories(parentPath);
        }

        nlohmann::json json;
        json["number"] = conv.number;

        for (const auto &msg : conv.messages)
        {
            json["messages"].push_back({{"message", msg.message},
                                        {"who", msg.who},
                                        {"date", msg.date}});
        }

        storage::Path path(filePath);
        storage::FileStream file;
        file.open(path.str(), storage::Mode::WRITE);

        if (!file.isopen())
        {
            std::cerr << "Failed to open file for writing: " << filePath << " Error: " << strerror(errno) << std::endl;
            return;
        }

        std::string jsonString = json.dump(4);
        std::cout << "Writing JSON: " << jsonString << std::endl;
        file.write(jsonString);
        file.close();
    }
}
