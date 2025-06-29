#include "standby.hpp"

#include "app.hpp"
#include "gpio.hpp"
#include "graphics.hpp"
#include "gsm2.hpp"
#include "hardware.hpp"

#include <LovyanGFX.hpp>
#include <clock.hpp>
#include <delay.hpp>
#include <libsystem.hpp>
#include <string>
#include <threads.hpp>

#ifdef ESP_PLATFORM

#include "driver/gpio.h" // Required for GPIO_NUM_15
#include "driver/uart.h" // Required for UART_NUM_2
#include "esp_sleep.h"
#include "freertos/task.h"

#endif

#define TICKS_MS 2

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
        if (enabled == true)
            return;
        lastTrigger = os_millis();

        graphics::setBrightness(graphics::getBrightness());
    }

    void triggerPower()
    {
        lastPowerTrigger = os_millis();

        if (powerMode == false /* && enabled == false*/)
            restorePower();
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
        if (Gsm::CallState() != Gsm::CallState::IDLE)
        {
            trigger();
            lastTrigger = os_millis();
            return;
        }

        if (!enabled && os_millis() - lastTrigger > sleepTime - 10000)
        {
            // Dim screen
            graphics::setBrightness(graphics::getBrightness() / 3 + 3, true);
        }

        if (os_millis() - lastPowerTrigger > 5000)
        {
            if (powerMode == true)
            {
                // savePower();
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

    void savePower()
    {
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

    void restorePower()
    {
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

    void lightSleepMillis(unsigned long t)
    {
#ifdef ESP_PLATFORM

        graphics::getLCD()->waitDMA();

        // Convert milliseconds to microseconds for the timer wake-up
        uint64_t time_us = (uint64_t) t * 1000;

        // --- Configure time management ---
        sleepStartTimeMillis = millis();

        // Get RTC time before sleep
        time_t rtcTimeBeforeSleep;
        time(&rtcTimeBeforeSleep);
        char rtcBeforeBuf[30];
        strftime(
            rtcBeforeBuf,
            sizeof(rtcBeforeBuf),
            "%Y-%m-%d %H:%M:%S",
            localtime(&rtcTimeBeforeSleep)
        );

        // --- Configure Wake-up Sources ---

        // 1. Timer Wake-up: Wake up after 't' milliseconds
        esp_sleep_enable_timer_wakeup(time_us);
        // printf("Timer wake-up enabled for %lu ms (%llu us)\n", t, time_us);

        // 2. External Wake-up (GPIO Ext0): Wake up when PIN_HOME_BUTTON (GPIO 15)
        // goes low GPIO 15 is an RTC GPIO and supported by ext0.
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 0); // Wake up on GPIO 15 LOW
        // printf("GPIO (Ext0) wake-up enabled on pin %d (LOW)\n", PIN_HOME_BUTTON);

        // 3. External Wake-up (GPIO Ext1): Wake up when RX pin (GPIO 26) goes low
        // This detects the start bit of serial data.
        // GPIO 26 is an RTC GPIO and supported by ext1.
        // We need to create a bitmask for the desired GPIO(s).
        uint64_t ext1_wakeup_mask = (1ULL << GPIO_NUM_26);
        // Configure Ext1 to wake up on ANY of the pins in the mask going LOW.
        // This is suitable for detecting the serial start bit (idle HIGH -> LOW).
        esp_sleep_enable_ext1_wakeup(ext1_wakeup_mask, (esp_sleep_ext1_wakeup_mode_t) 0);
        // printf("GPIO (Ext1) wake-up enabled on pin %d (ANY_LOW - detects serial
        // start bit)\n", RX);

        // --- Enter Light Sleep ---
        // printf("Entering Light Sleep...\n");

        // Before sleeping, ensure any pending Serial output is sent
        Serial.flush();
        Serial2.flush(); // Also flush Serial2 if you were sending data on it

        // Enter light sleep mode
        esp_light_sleep_start();

        // --- After Wake-up ---
        // Execution resumes here after waking up.
        // printf("Woke up!\n"); // This should print after waking

        // Optional: Check the wake-up cause
        esp_sleep_wakeup_cause_t wakeup_cause = esp_sleep_get_wakeup_cause();
        switch (wakeup_cause)
        {
        case ESP_SLEEP_WAKEUP_TIMER:
            // printf("Wakeup cause: Timer\n");
            break;
        case ESP_SLEEP_WAKEUP_EXT0:
            // printf("Wakeup cause: External signal (GPIO 15)\n");
            break;
        case ESP_SLEEP_WAKEUP_EXT1:
            // printf("Wakeup cause: External signal (GPIO 26 - likely serial
            // data)\n");
            //  Note: Data might already be in the buffer.
            //  You might want to read it here or in your loop.
            libsystem::setDeviceMode(libsystem::NORMAL);
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
            // printf("Wakeup cause: Undefined (e.g., reset)\n");
            break;
        default:
            // printf("Wakeup cause: Other (%d)\n", wakeup_cause);
            break;
        }
        // It's good practice to disable wake-up sources if you don't immediately
        // go back to sleep, although they are typically cleared on reset/deep
        // sleep. For light sleep and immediate re-entry, you might not need to
        // disable. esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

        // Get RTC time after wake-up
        time_t rtcTimeAfterWakeup;
        time(&rtcTimeAfterWakeup);
        char rtcAfterBuf[30];
        strftime(
            rtcAfterBuf,
            sizeof(rtcAfterBuf),
            "%Y-%m-%d %H:%M:%S",
            localtime(&rtcTimeAfterWakeup)
        );
        // printf("RTC Time After Wakeup: %s (%ld)\n", rtcAfterBuf,
        // rtcTimeAfterWakeup);

        double sleepDurationSeconds = difftime(rtcTimeAfterWakeup, rtcTimeBeforeSleep);
        unsigned long actualSleepDurationMillis =
            static_cast<unsigned long>(sleepDurationSeconds / 1000);

        // Calculate apparent sleep duration using millis
        unsigned long apparentSleepDurationMillis = millis() - sleepStartTimeMillis;

        // Calculate the correction offset
        sleepTimeCorrectionOffset += apparentSleepDurationMillis;
#endif
    }

    void sleepCycle()
    {
#ifdef ESP_PLATFORM

        if (hardware::vibrator::isPlaying())
        {
            PaxOS_Delay(50);
            return;
        }

        std::cout << "Sleep cycle" << std::endl;
        while (!buisy_io.try_lock())
        {
            PaxOS_Delay(1);
            if (hardware::getHomeButton())
            {
                libsystem::setDeviceMode(libsystem::NORMAL);
                return;
            }
        }

        if (hardware::vibrator::isPlaying())
        {
            buisy_io.unlock();
            for (int i = 0; i < 100; i++)
            {
                PaxOS_Delay(1);
                if (hardware::getHomeButton())
                {
                    libsystem::setDeviceMode(libsystem::NORMAL);
                    return;
                }
            }
            return;
        }

        lightSleepMillis(3000);
        buisy_io.unlock();
        Gsm::checkForMessages();
#endif
    }

    void wait()
    {
        // printf("wait begin\n");
        update();

        static uint64_t timer = os_millis();

        uint64_t dt = os_millis() - timer;

        if (dt < TICKS_MS)
        {
            uint64_t tw = TICKS_MS - dt;

#ifdef ESP_PLATFORM
            vTaskDelay(pdMS_TO_TICKS(tw));
#else
            SDL_Delay(tw);
#endif
        }
        else
#ifdef ESP_PLATFORM
            vTaskDelay(pdMS_TO_TICKS(1));
#else
            SDL_Delay(1);
#endif

#ifdef ESP_PLATFORM
// std::cout << "CPU USAGE: " << (float) (100*(dt/TICKS_MS)) << "%" <<
// std::endl;
#endif

        timer = os_millis();
        // printf("wait end\n");
    }
} // namespace StandbyMode
