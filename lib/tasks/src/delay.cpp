#include "delay.hpp"
#include <standby.hpp>

#ifdef ESP_PLATFORM
#include <LovyanGFX.hpp>
#else
#include <SDL2/SDL.h>
#endif

void PaxOS_Delay(int64_t ms) {
  #ifdef ESP_PLATFORM
    vTaskDelay(pdMS_TO_TICKS(ms));
  #else
    SDL_Delay(ms);
  #endif
}
