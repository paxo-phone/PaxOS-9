#include <iostream>
#include <list>
#include "event.cpp"

/**
 * class eventManager 
 *  - implemente le pattern Singleton -
 * 
 * permet de manipuler les événements applicatifs 
 * et lancer les actions lorsque le timer arrive à son terme
 **/ 

using namespace std;

class eventManager {

    protected:
        static eventManager* instance; //= nullptr;

        list <event> lstEvents;


    public:
        // methode principale pour récupérer l'instance de l'eventManager
        static eventManager* getInstance() {
            if (instance == nullptr) {
                instance = new eventManager();
            }
            return instance;
        }

    // eventManager should not be cloneable.
    eventManager(eventManager &other) = delete;

     //eventManager should not be assignable.
    void operator=(const eventManager &) = delete;

    private:
        // default constructor
        eventManager () {}


    

};