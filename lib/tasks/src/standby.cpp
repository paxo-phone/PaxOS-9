#include "standby.hpp"

#include <threads.hpp>

#ifdef ESP_PLATFORM

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <esp_system.h>

#endif

namespace StandbyMode
{
    uint64_t lastTrigger = millis();
    uint64_t sleepTime = 30000;
    bool enabled = false;

    void trigger()
    {
        lastTrigger = millis();
    }

    void update()
    {
        if (millis() - lastTrigger > 1000)
        {
            enabled = true;
        }
    }

    void setSleepTime(uint64_t sleepTime)
    {
        StandbyMode::sleepTime = sleepTime;
    }

    bool state()
    {
        return enabled;
    }
    
    void enable()
    {
        enabled = true;
    }
    
    void disable()
    {
        enabled = false;
        lastTrigger = millis();
    }

    void savePower()
    {
        #ifdef ESP_PLATFORM
        setCpuFrequencyMhz(20);
        GSM::reInit();
        #endif
    }

    void restorePower()
    {
        #ifdef ESP_PLATFORM
        setCpuFrequencyMhz(240);
        GSM::reInit();
        #endif
    }

    void wait()
    {
        #ifdef ESP_PLATFORM
        static uint64_t timer = millis();

        uint64_t dt = millis() - timer;

        if(dt < 50)
        {
            uint64_t tw = 50 - dt;

            vTaskDelay(pdMS_TO_TICKS(tw));
        }

        timer = millis();
        #endif
    }
}