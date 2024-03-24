#include "hardware.hpp"
#include "gpio.hpp"

#ifdef ESP_PLATFORM
    #include <Arduino.h>
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

    #endif

    return false;
}