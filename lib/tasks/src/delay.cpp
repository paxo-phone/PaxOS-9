#include "delay.hpp"

#include <iostream>
#include <standby.hpp>

#ifdef ESP_PLATFORM
#include <LovyanGFX.hpp>
#else
#if __has_include(<SDL2/SDL.h>)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#endif

void PaxOS_Delay(int64_t ms)
{
#ifdef ESP_PLATFORM
    // vTaskDelay(pdMS_TO_TICKS(ms));

    uint64_t start = esp_timer_get_time();
    uint64_t end = start + (ms * 1000); // Convert ms to us

    while (esp_timer_get_time() < end)
    {
        // Yield to other tasks
        vTaskDelay(1); // Delay for 1 tick, allowing other tasks to run
    }
#else
    SDL_Delay(ms);
#endif
}
