#include "message.hpp"

#include <fstream>
#include <json.hpp>

namespace Message
{
    void loadMessages(const std::string& filePath, std::vector<Message>& messages)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            return;

        std::string content(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        file.close();

        nlohmann::json json = nlohmann::json::parse(content);

        for (const auto& item : json)
        {
            Message message;
            message.number = item["number"].get<std::string>();
            message.message = item["message"].get<std::string>();
            message.date = item["date"].get<std::string>();
            messages.push_back(message);
        }
    }

    void saveMessages(const std::string& filePath, const std::vector<Message>& messages)
    {
        nlohmann::json json;
        for (const auto& message : messages)
        {
            json.push_back(
                {{"number", message.number}, {"message", message.message}, {"date", message.date}}
            );
        }

        std::ofstream file(filePath);
        if (!file.is_open())
            return;

        file << json.dump(4);
        file.close();
    }
} // namespace Message
