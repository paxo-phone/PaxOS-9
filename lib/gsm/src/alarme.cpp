#include "alarme.hpp"
#include "gsm.hpp"
#include <fstream>
#include <json.hpp>
#include "GuiManager.hpp"

    static void loadAlarmes(const std::string& filePath, std::vector<Alarme::Alarme>& alarmes)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
            return;

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        nlohmann::json json = nlohmann::json::parse(content);

        for (const auto& item : json)
        {
            Alarme::Alarme alarme;
            alarme.heure = item["heure"].get<std::string>();
            alarme.nom = item["nom"].get<std::string>();
            alarme.actif = item["actif"].get<bool>();
            alarmes.push_back(alarme);
        }
    }

    void saveAlarme(const std::string& filePath, const std::vector<Alarme>& alarmes)
    {
        nlohmann::json json;
        for (const auto& message : alarmes)
        {
            json.push_back({{"heure", message.heure}, {"nom", message.nom}, {"actif", message.actif}});
        }

        std::ofstream file(filePath);
        if (!file.is_open())
            return;

        file << json.dump(4);
        file.close();
    }



void Alarme::ringAlarme(){
        GuiManager &guiManager = GuiManager::getInstance();
        guiManager.showInfoMessage(this.nom);
}


/**
 * setAlarme
 */
    uint8_t Alarme::setAlarme (std::string strHeure, std::string strNom, bool bActif){

        //Alarme alarme = {heure, nom, actif};

        actif = bActif;


        if (actif){
            return  eventHandlerApp.addEventListener(
                new Callback<>(checkAlarme(alarme)),
                new Callback<>([](){checkAlarme(alarme);}));
        }
        return -1;
    }

    /**
     * checklAlarme
     * vérifie l'heure de l'alarme
     */
    bool checkAlarme() {
        

        
        if (actif) {
            try {
                std::string strHeure = alarme.heure;
                int pos = strHeure.find(":");
                u_int8_t heure = stoi(strHeure.substr(0, pos)) ;
                u_int8_t minute = stoi(strHeure.substr(pos)) ;

                return GSM::hours == heure && GSM::minutes == minute && GSM::seconds == 0;
            }
            catch (...){
                return false;
            }
        }
        else
            return false;

    }


