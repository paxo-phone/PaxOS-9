#ifndef LAUNCHER_HPP
#define LAUNCHER_HPP

#include <string>
#include <vector>
#include <gui.hpp>
#include <hardware.hpp>

// application launcher en une fonction. Lorsqu'une app est choisie, il l'ajoute a la liste des programes a executer et quitte.
// et le thread app l'éxécute immédiatement.

namespace AppManager {
    class App;
}

namespace applications::launcher {
    void init();
    void free();

    void update();
    void draw();

    bool iconTouched();
    std::shared_ptr<AppManager::App> getApp();
}

int launcher();

#endif