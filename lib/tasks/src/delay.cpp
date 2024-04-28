#include "delay.hpp"
#include <standby.hpp>

#include <LovyanGFX.hpp>

void PaxOS_Delay(int64_t ms) {
  #ifdef ESP_PLATFORM
    vTaskDelay(pdMS_TO_TICKS(ms));
  #else
    SDL_Delay(ms);
  #endif
}