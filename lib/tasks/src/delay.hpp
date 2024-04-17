#pragma once

#include <LovyanGFX.hpp>

inline void PaxOS_Delay(int64_t ms) {
  #ifdef ESP_PLATFORM
    vTaskDelay(pdMS_TO_TICKS(ms));
  #else
    SDL_Delay(ms);
  #endif
}
