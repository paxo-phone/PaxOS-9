#include "hardware.hpp"
#include "gpio.hpp"

#ifdef ESP_PLATFORM
    #include <Arduino.h>
#else
    #ifdef __linux__
        #include <SDL2/SDL.h>
    #else
        #include <SDL_keyboard.h>
    #endif
#endif

void hardware::init()
{
    #ifdef ESP_PLATFORM

    psramInit();
    pinMode(PIN_SCREEN_POWER, OUTPUT);
    pinMode(PIN_VIBRATOR, OUTPUT);
    pinMode(PIN_HOME_BUTTON, INPUT_PULLUP);
    setVibrator(false);

    #endif
}

void hardware::setScreenPower(bool power)
{
    #ifdef ESP_PLATFORM

    digitalWrite(PIN_SCREEN_POWER, power);

    #endif
}

void hardware::setVibrator(bool state)
{
    #ifdef ESP_PLATFORM

    digitalWrite(PIN_VIBRATOR, state);

    #endif
}

bool hardware::getHomeButton()
{
    #ifdef ESP_PLATFORM

        return !digitalRead(PIN_HOME_BUTTON);

    #else

        return (SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_ESCAPE] || SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_Q]);
    
    #endif
}