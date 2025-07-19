#ifndef LAUNCHER_HPP
#define LAUNCHER_HPP

#include <gui.hpp>
#include <hardware.hpp>
#include <string>
#include <vector>

// application launcher en une fonction. Lorsqu'une app est choisie, il l'ajoute a la liste des
// programes a executer et quitte. et le thread app l'éxécute immédiatement.

namespace AppManager
{
    class App;
}

namespace applications::launcher
{
    void init(); // load the launcher
    void free(); // free the launcher

    void update(); // update the launcher gui and events
    void draw();   // make the gui of the launcher

    bool iconTouched();                        // check if an app icon is touched
    std::shared_ptr<AppManager::App> getApp(); // get the selected app
} // namespace applications::launcher

#endif
