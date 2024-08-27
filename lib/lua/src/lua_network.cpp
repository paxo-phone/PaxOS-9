#include "lua_network.hpp"


    /**
     * fonction principale
     * Appelle une URL
     */
    void LuaNetwork::callURL(std::string url, std::function <void(std::string)> completionHandler) {


            // Récupération de l'instance de Session
            network::URLSessionDataTask* getTask = network::URLSession::defaultInstance.get()->dataTaskWithURL(
                network::URL(url), 
                [completionHandler](const std::string& data) {
                {
                    // std::cout << data << std::endl;
                    completionHandler(data);
                }
            });


  
            // call back fonction pour mise à jour de l'avancement de la requete
           getTask->downloadProgressHandler = [&](double progress)
            {
//                std::cout << "Received progress " << progress << std::endl;
            };

            // Lancement de la requete
            getTask->resume();

    }
