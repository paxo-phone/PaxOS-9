#ifndef APP_FILE
#define APP_FILE

#include <json.hpp>
#include <string>
#include <vector>

#define PATH_LOCATION "./"

using json = nlohmann::json;

namespace appFile
{
    std::string load(std::string filename);
    json parse(std::string str);
    void save(std::string filename, json jsonObj);
}; // namespace appFile

#endif // APP_FILE
