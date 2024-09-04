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
        lastTrigger = millis();

        graphics::setBrightness(graphics::brightness);
    }

    void triggerPower()
    {
        lastPowerTrigger = millis();

        if(powerMode == false/* && enabled == false*/)
        {
            restorePower();
        }
    }

    void update()
    {
        if (!enabled && millis() - lastTrigger > sleepTime - 10000)
        {
            graphics::setBrightness(graphics::brightness/3 + 3);
        }

        if (!enabled && millis() - lastTrigger > sleepTime)
        {
            enable();
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
        lastTrigger = millis() - sleepTime;

        graphics::setBrightness(0);
        StandbyMode::savePower();

        while(AppManager::isAnyVisibleApp())
        {
            AppManager::appStack[AppManager::appStack.size() - 1]->kill();
            AppManager::appStack.pop_back();
        }

        while (hardware::getHomeButton());
        while (!hardware::getHomeButton() && !AppManager::isAnyVisibleApp() && millis() - sleepTime > lastTrigger)
        {
            eventHandlerApp.update();
            AppManager::loop();
        }

        while (hardware::getHomeButton());
        
        StandbyMode::restorePower();
        graphics::setBrightness(graphics::brightness);

        disable();
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