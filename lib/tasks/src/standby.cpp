#include "standby.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "hardware.hpp"
#include "gsm.hpp"

#include <threads.hpp>
#include <LovyanGFX.hpp>

#ifdef ESP_PLATFORM

#include "freertos/task.h"

#endif

#include <gsm.hpp>

#define TICKS_MS 20

namespace StandbyMode
{
    uint64_t lastTrigger = millis();
    uint64_t lastPowerTrigger = millis();
    uint64_t sleepTime = 30000;
    bool enabled = false;
    bool powerMode = true; // false is low, true is high

    void trigger()
    {
        if(enabled == true)
            return;
        lastTrigger = millis();

        graphics::setBrightness(graphics::getBrightness());
    }

    void triggerPower()
    {
        lastPowerTrigger = millis();

        if(powerMode == false/* && enabled == false*/)
        {
            restorePower();
        }
    }

    bool expired()
    {
        return millis() - lastTrigger > sleepTime;
    }

    void reset()
    {
        lastTrigger = millis();
    }

    void update()
    {
        if (!enabled && millis() - lastTrigger > sleepTime - 10000)
        {
            // Dim screen
            graphics::setBrightness(graphics::getBrightness()/3 + 3, true);
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
        lastTrigger = millis();
    }
    
    void disable()
    {
        enabled = false;
        lastTrigger = millis();
    }

    void savePower()
    {
        #ifdef ESP_PLATFORM
        Serial.end();
        setCpuFrequencyMhz(20);
        GSM::reInit();
        Serial.begin(115200);
        powerMode = false;
        #endif
    }

    void restorePower()
    {
        #ifdef ESP_PLATFORM
        Serial.end();
        setCpuFrequencyMhz(240);
        GSM::reInit();
        Serial.begin(115200);
        powerMode = true;
        #endif
    }

    void wait()
    {
        update();

        static uint64_t timer = millis();

        uint64_t dt = millis() - timer;

        if(dt < TICKS_MS)
        {
            uint64_t tw = TICKS_MS - dt;

        #ifdef ESP_PLATFORM
            vTaskDelay(pdMS_TO_TICKS(tw));
        #else
            SDL_Delay(tw);
        #endif
        }

        #ifdef ESP_PLATFORM
        //std::cout << "CPU USAGE: " << (float) (100*(dt/TICKS_MS)) << "%" << std::endl;
        #endif

        timer = millis();
    }
}