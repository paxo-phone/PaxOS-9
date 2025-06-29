#ifndef THREADS_HPP
#define THREADS_HPP

#include <tasks.hpp>

extern EventHandler eventHandlerBack;
extern EventHandler eventHandlerApp;

#define CORE_APP 1
#define CORE_BACK 0

namespace ThreadManager
{
    void init();
    void new_thread(bool core, void (*func)(void*), int stackSize = 10000);

    void background_thread(void* arg);
    void simcom_thread(void* arg);
    void app_thread();
}; // namespace ThreadManager

#endif
