#include "delay.hpp"
#include <standby.hpp>

#ifdef ESP_PLATFORM
#include <LovyanGFX.hpp>
#else
#if defined(__linux__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif
#endif

void PaxOS_Delay(int64_t ms) {
  #ifdef ESP_PLATFORM
    vTaskDelay(pdMS_TO_TICKS(ms));
  #else
    SDL_Delay(ms);
  #endif
}
