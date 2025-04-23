#include "standby.hpp"
#include "app.hpp"
#include "graphics.hpp"
#include "hardware.hpp"
#include "gsm.hpp"

#include <threads.hpp>
#include <LovyanGFX.hpp>
#include <clock.hpp>
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
    uint64_t lastTrigger = os_millis();
    uint64_t lastPowerTrigger = os_millis();
    uint64_t sleepTime = 30000;
    bool enabled = false;
    bool powerMode = true; // false is low, true is high

    void trigger()
    {
        if(enabled == true)
            return;
        lastTrigger = os_millis();

        graphics::setBrightness(graphics::getBrightness());
    }

    void triggerPower()
    {
        lastPowerTrigger = os_millis();

        if(powerMode == false/* && enabled == false*/)
        {
            restorePower();
        }
    }

    bool expired()
    {
        return os_millis() - lastTrigger > sleepTime;
    }

    void reset()
    {
        lastTrigger = os_millis();
    }

    void update()
    {
        if (!enabled && os_millis() - lastTrigger > sleepTime - 10000)
        {
            // Dim screen
            graphics::setBrightness(graphics::getBrightness()/3 + 3, true);
        }

        if (os_millis() - lastPowerTrigger > 5000)
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
        lastTrigger = os_millis();
    }
    
    void disable()
    {
        enabled = false;
        lastTrigger = os_millis();
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
        /*
            Cette fonction a été développée par Gemini (je n'ai pas réussi a la faire moi même; Si quelqu'un propose mieux :) )
            Elle a pour but de créer une pause courte de 1/4s pour économiser de l'énergie, et doit aussi corriger le décalage de delais de millis() tout en gérant quand le bloquage doit avoir lieu en fonction des autres coeurs et threads
        */

        if(!buisy_io.try_lock())
            return;

        Serial.flush();
        //printf("sleep mode\n");

        // Record millis before sleep
        sleepStartTimeMillis = millis();

        // Get RTC time before sleep
        time_t rtcTimeBeforeSleep;
        time(&rtcTimeBeforeSleep);
        char rtcBeforeBuf[30];
        strftime(rtcBeforeBuf, sizeof(rtcBeforeBuf), "%Y-%m-%d %H:%M:%S", localtime(&rtcTimeBeforeSleep));
        //printf("RTC Time Before Sleep: %s (%ld)\n", rtcBeforeBuf, rtcTimeBeforeSleep);

        esp_sleep_enable_timer_wakeup(250000);

        // Enable external wake-up on the specified pin (falling edge in this case)
        esp_sleep_enable_ext0_wakeup(gpio_num_t(PIN_HOME_BUTTON), 0); // 0 for low level (pulled down)

        // Enter light sleep
        esp_light_sleep_start();

        // Code execution resumes here after waking up
        //printf("Woke up!\n");

        // Get RTC time after wake-up
        time_t rtcTimeAfterWakeup;
        time(&rtcTimeAfterWakeup);
        char rtcAfterBuf[30];
        strftime(rtcAfterBuf, sizeof(rtcAfterBuf), "%Y-%m-%d %H:%M:%S", localtime(&rtcTimeAfterWakeup));
        //printf("RTC Time After Wakeup: %s (%ld)\n", rtcAfterBuf, rtcTimeAfterWakeup);

        double sleepDurationSeconds = difftime(rtcTimeAfterWakeup, rtcTimeBeforeSleep);
        unsigned long actualSleepDurationMillis = static_cast<unsigned long>(sleepDurationSeconds / 1000);

        // Calculate apparent sleep duration using millis
        unsigned long apparentSleepDurationMillis = millis() - sleepStartTimeMillis;

        // Calculate the correction offset
        sleepTimeCorrectionOffset += apparentSleepDurationMillis;

        //printf("Apparent sleep duration (millis): %lu ms\n", apparentSleepDurationMillis);
        //printf("Actual sleep duration (RTC): %lu ms\n", actualSleepDurationMillis);
        //printf("Time correction offset: %lld ms\n", sleepTimeCorrectionOffset);

        // You can check the reason for wake-up if needed
        esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

        switch (wakeup_reason) {
            case ESP_SLEEP_WAKEUP_TIMER:
            //printf("Woke up due to timer.\n");
            break;
            case ESP_SLEEP_WAKEUP_EXT0:
            //printf("Woke up due to external signal on GPIO %d\n", PIN_HOME_BUTTON);
            break;
            default:
            //printf("Woke up due to other reason: %d\n", wakeup_reason);
            break;
        }

        Serial.flush();
        buisy_io.unlock();
        GSM::unlockSemaphore();
    }


    void wait()
    {
        //printf("wait begin\n");
        update();

        static uint64_t timer = os_millis();

        uint64_t dt = os_millis() - timer;

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

        timer = os_millis();
        //printf("wait end\n");
    }
}