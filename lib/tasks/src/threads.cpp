#include "threads.hpp"
#include "delay.hpp"
#include <clock.hpp>

#ifndef THREAD_HANDLER
    #define THREAD_HANDLER
    EventHandler eventHandlerBack;
    EventHandler eventHandlerApp;
#endif

#ifdef ESP_PLATFORM
    #include <Arduino.h>
    #include "soc/rtc_wdt.h"
    #include "esp_heap_caps.h"
    #include <esp_task_wdt.h>
#endif

#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

#include <thread>

#endif

void ThreadManager::init()
{
    new_thread(CORE_BACK, &ThreadManager::simcom_thread, 16*1024);
    new_thread(CORE_BACK, &ThreadManager::background_thread, 16*1024);
}

void ThreadManager::new_thread(bool core, void(*func)(void*), int stackSize)
{
    #ifdef ESP_PLATFORM
        xTaskCreate(func,
                    "thread",
                    stackSize,
                    nullptr,
                    0,
                    nullptr);
    #else
        std::thread myThread(func, nullptr);
        myThread.detach();
    #endif
}

void ThreadManager::background_thread(void* arg)
{
    while (true)
    {
      eventHandlerBack.update();
      PaxOS_Delay(10);
    }
}

void ThreadManager::simcom_thread(void* arg)
{
    GSM::run();
}

void ThreadManager::app_thread(){}
