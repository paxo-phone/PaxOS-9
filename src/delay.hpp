//
// Created by Charlito33 on 02/01/2024.
// Copyright (c) 2024 Charlito33. All rights reserved.
//

#ifndef DELAY_HPP
#define DELAY_HPP

// PLEASE MOVE IT TO "threads" (that should be called "threading" to be honest...)
namespace temp
{
    inline void delay(int ms)
    {
#ifdef ESP_PLATFORM
        vTaskDelay(pdMS_TO_TICKS(ms));
#else
        SDL_Delay(ms);
#endif
    }
}

#endif //DELAY_HPP
