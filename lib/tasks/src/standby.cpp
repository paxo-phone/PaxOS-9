#include "standby.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "hardware.hpp"
#include "gsm.hpp"

#include <threads.hpp>
#include <LovyanGFX.hpp>
#include "gpio.hpp"
#include <string>

#ifdef ESP_PLATFORM

#include "freertos/task.h"

#endif

#include <gsm.hpp>

#define TICKS_MS 20

namespace StandbyMode
{
    std::mutex buisy_io;
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
                //savePower();
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

    void savePower() {
        /*printf("Save Power --------------------------------------------\n");
        //buisy_io.lock();
        //portDISABLE_INTERRUPTS(); // Disable interrupts
        Serial.end();
        //esp_task_wdt_reset();
        setCpuFrequencyMhz(20);
        GSM::reInit();
        Serial.begin(115200);
        //portENABLE_INTERRUPTS();  // Re-enable interrupts
        powerMode = false;
        //buisy_io.unlock();*/
    }

    void restorePower() {
        /*printf("Restore Power --------------------------------------------\n");
        //buisy_io.lock();
        //portDISABLE_INTERRUPTS(); // Disable interrupts
        Serial.end();
        //esp_task_wdt_reset();
        setCpuFrequencyMhz(240);
        GSM::reInit();
        Serial.begin(115200);
        //portENABLE_INTERRUPTS();  // Re-enable interrupts
        powerMode = true;
        //buisy_io.unlock();*/
    }

    void sleepCycle()
    {
        if(!buisy_io.try_lock())
            return;

        Serial.flush();
        printf("sleep mode\n");
        esp_sleep_enable_timer_wakeup(250000);

        // Enable external wake-up on the specified pin (falling edge in this case)
        esp_sleep_enable_ext0_wakeup(gpio_num_t(PIN_HOME_BUTTON), 0); // 0 for low level (pulled down)

        // Enter light sleep
        esp_light_sleep_start();

        // Code execution resumes here after waking up
        printf("Woke up!");

        // You can check the reason for wake-up if needed
        esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

        switch (wakeup_reason) {
            case ESP_SLEEP_WAKEUP_TIMER:
            printf("Woke up due to timer.\n");
            break;
            case ESP_SLEEP_WAKEUP_EXT0:
            printf("Woke up due to external signal on GPIO\n");
            break;
            default:
            printf("Woke up due to other reason: \n");
            break;
        }

        Serial.flush();
        buisy_io.unlock();
    }

    void wait()
    {
        //printf("wait begin\n");
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
        else
            vTaskDelay(pdMS_TO_TICKS(1));

        #ifdef ESP_PLATFORM
        //std::cout << "CPU USAGE: " << (float) (100*(dt/TICKS_MS)) << "%" << std::endl;
        #endif

        timer = millis();
        //printf("wait end\n");
    }
}