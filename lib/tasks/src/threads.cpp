#include "threads.hpp"

#include "delay.hpp"

#include <clock.hpp>
#include <gsm2.hpp>
#include <network.hpp>

#ifndef THREAD_HANDLER
#define THREAD_HANDLER
EventHandler eventHandlerBack;
EventHandler eventHandlerApp;
#endif

#ifdef ESP32
#include "esp_heap_caps.h"
#include "soc/rtc_wdt.h"

#include <Arduino.h>
#include <esp_task_wdt.h>
#endif

#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

#include <thread>

#endif

void ThreadManager::init()
{
    new_thread(CORE_BACK, &ThreadManager::simcom_thread, 8 * 1024);
    new_thread(CORE_BACK, &ThreadManager::background_thread, 4 * 1024);
    new_thread(CORE_BACK, &Network::network_thread, 12 * 1024);
}

void ThreadManager::new_thread(bool core, void (*func)(void*), int stackSize)
{
#ifdef ESP_PLATFORM
    xTaskCreatePinnedToCore(func, "thread", stackSize, NULL, 5, NULL, core);
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
    // GSM::run();
    Gsm::init();
    printf("[GSM] GSM initialized\n");
    Gsm::loop();
}

void ThreadManager::app_thread() {}
