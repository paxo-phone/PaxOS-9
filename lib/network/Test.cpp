#include "network.hpp"


namespace network {

    void testURL() {

        // Connexion
        // connect(const std::string& ssid, const std::string& password)
         //NetworkManager connexion = NetworkManager::sharedInstance; 
        // connexion->connect(const std::string& ssid, const std::string& password)
        NetworkManager->connect("wifiMaison","Ur2kewl4mi!*");

        // création de l'url
        URL url = new URL("https://api.open-meteo.com/v1/forecast?latitude=48.8534&longitude=2.3488&hourly=temperature_2m,precipitation_probability,precipitation,weather_code,wind_speed_10m&forecast_hours=24&timezone=aut");
        URLRequest request(url);

        URLSessionDataTask session = new URLSessionDataTask(request, requestOK);
        session->resume();

    }

    void requestOK(std::string response) {
        // std::cout << session->responseData
        std::cout << "request OK" <<std::endl:
        std::cout << response <<std::endl:

    }

}