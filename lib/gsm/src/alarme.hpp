#ifndef ALARME_HPP
#define ALARME_HPP

#include <vector>
#include <string>
#include <json.hpp>

namespace Alarme
{
    class Alarme{
        public:
            static void loadAlarme(const std::string& filePath, std::vector<Alarme>& alarme);
            
            void saveAlarme(const std::string& filePath, const std::vector<Alarme>& alarme);
            bool checkAlarme();
            uint8_t setAlarme (std::string heure, std::string nom, bool actif);
            void ringAlarme();


        private:
            std::string heure;
            std::string nom;
            bool actif;
    };

    struct Alarme {
            std::string heure;
            std::string nom;
            bool actif;
    }

}

#endif
