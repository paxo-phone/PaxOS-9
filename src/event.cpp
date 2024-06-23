#include <string>
#include <functional>

// utilisation du namespace std dans la définition de la classe event
using namespace std;

/**
 * classe event
 * porte la définition d'un événement
 */
class event {

    public:
        event(string id_, string comment_, int timeout, function <void()> callback_);
        string getId() {
            return id;
        }

        string getComment() {
            return comment;
        }

        int getTimeOut() {
            return timeout;
        }

        void launchCallback() {
            callback;
        }

    private: 
        string id;
        string comment;
        int timeout;
        function <void()> callback;

};

// default constructor
event::event(string id_, string comment_, int timeout_, function<void()> callback_) {
    id = id_;
    comment = comment_;
    timeout = timeout_;
    callback = callback_;
}
