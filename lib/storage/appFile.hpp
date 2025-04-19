#ifndef APP_FILE
#define APP_FILE

#include <string>
#include <vector>
#include <json.hpp>

#define PATH_LOCATION "./"

using json = nlohmann::json;

namespace appFile
{
    std::string load(std::string filename);
    json parse(std::string str);
    void save(std::string filename, json jsonObj);
};

#endif // APP_FILE
