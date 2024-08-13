#include "appFile.hpp"
#include <filestream.hpp>
#include <path.hpp>
#include <json.hpp>
#include <iostream>



namespace appFile
{
    // Liste d'objets
    std::string load(std::string filename)
    {
        // check sur le filename

        // vérifie que le filename est bien uniquement dans le répertoire de l'app
        storage::Path path(PATH_LOCATION);
        storage::FileStream stream((path / filename).str(), storage::Mode::READ);
        std::string file = stream.read();
        stream.close();

        return file;
    }

    json parse(std::string str){

        nlohmann::json json;
        if (nlohmann::json::accept(str)) {
            try {
                json = nlohmann::json::parse(str);
                return json;
            }
            catch (const nlohmann::json::exception &e)
            {
                std::cerr << "Error parsing generic Obj: " << e.what() << std::endl;
            }
        }
        else {
                std::cerr << "Parsing file " << str << " failed" << std::endl;
        }
        return NULL;
    }

    void save(std::string filename, json jsonObj)
    {

        std::string file = jsonObj.dump();

        storage::Path path(PATH_LOCATION);
        storage::FileStream stream((path / filename).str(), storage::Mode::WRITE);
        stream.write(file);
        stream.close();
    }


};
