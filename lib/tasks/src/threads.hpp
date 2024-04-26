#ifndef THREADS_HPP
#define THREADS_HPP

#include "tasks.hpp"
#include "standby.hpp"

#include <gsm.hpp>

extern EventHandler eventHandlerBack;
extern EventHandler eventHandlerApp;

#include <stdint.h>
#include <string.h>

#define CORE_APP 1
#define CORE_BACK 0



namespace ThreadManager
{
    void init();
    void new_thread(bool core, void(*func)(void*));

    void background_thread(void* arg);
    void simcom_thread(void* arg);
    void app_thread();
};

#endif