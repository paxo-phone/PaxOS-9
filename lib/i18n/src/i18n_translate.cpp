#include <string>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <filesystem>
#include <iostream>

#include "i18n_translate.hpp"
#include "i18n_config.hpp"

namespace i18n
{

    std::unordered_map<std::string, std::string> gTranslations = {};

    inline void ltrim(std::string &str)
    {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char c)
                                            { return !std::isspace(c); }));
    }

    inline void rtrim(std::string &str)
    {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](char c)
                               { return !std::isspace(c); })
                      .base(),
                  str.end());
    }

    inline void trim(std::string &str)
    {
        rtrim(str);
        ltrim(str);
    }

    std::string normalizeNewlines(const std::string &str)
    {
        std::string result = str;
        size_t pos = 0;
        while ((pos = result.find("\r\n", pos)) != std::string::npos)
        {
            result.replace(pos, 2, "\n");
            pos += 1;
        }
        return result;
    }

    void setTextDomain(const std::string &domain)
    {
        gTranslations.clear();

        std::string path = domain + "." + langToString(getLang()) + ".i18n";
        std::ifstream file(path);
        if (file.is_open())
        {
            bool inquotes = false;
            std::string key = "";
            std::string temp = "";
            char c;

            while (file.get(c))
            {
                if (c == '"')
                {
                    inquotes = !inquotes;
                    continue;
                }

                if (inquotes)
                {
                    temp += c;
                    continue;
                }

                if (c == '=' && key.empty())
                {
                    key = temp;
                    trim(key);
                    temp.clear();
                    continue;
                }

                if (c == '\n')
                {
                    trim(temp);
                    temp = normalizeNewlines(temp);
                    gTranslations[key] = temp;
                    temp.clear();
                    key.clear();
                    continue;
                }

                temp += c;
            }

            if (!key.empty())
            {
                trim(temp);
                temp = normalizeNewlines(temp);
                gTranslations[key] = temp;
            }

            file.close();
        }
        else
        {
            std::cerr << "Unable to open file: " << path << std::endl;
        }
    }

    std::string getText(const std::string &key)
    {
        auto it = gTranslations.find(key);
        if (it != gTranslations.end())
        {
            return it->second;
        }
        else
        {
            std::cerr << "Translation not found for key: " << key << std::endl;
            return key;
        }
    }

}
