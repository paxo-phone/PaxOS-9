#include "standby.hpp"

#include <threads.hpp>
#include <LovyanGFX.hpp>

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
    uint64_t lastPowerTrigger = millis();
    uint64_t sleepTime = 30000;
    bool enabled = false;
    bool powerMode = true; // false is low, true is high

    void trigger()
    {
        lastTrigger = millis();
    }

    void triggerPower()
    {
        lastPowerTrigger = millis();

        if(powerMode == false)
        {
            restorePower();
        }
    }

    void update()
    {
        if (millis() - lastTrigger > 1000)
        {
            enabled = true;
        }

        if (millis() - lastPowerTrigger > 5000)
        {
            if(powerMode == true)
            {
                savePower();
            }
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
        powerMode = false;
        #endif
    }

    void restorePower()
    {
        #ifdef ESP_PLATFORM
        setCpuFrequencyMhz(240);
        GSM::reInit();
        powerMode = true;
        #endif
    }

    void wait()
    {
        update();

        static uint64_t timer = millis();

        uint64_t dt = millis() - timer;

        if(dt < 50)
        {
            uint64_t tw = 50 - dt;

        #ifdef ESP_PLATFORM
            vTaskDelay(pdMS_TO_TICKS(tw));
        #else
            SDL_Delay(tw);
        #endif
        }

        timer = millis();
    }
}