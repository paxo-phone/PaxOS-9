#include "hardware.hpp"

#include <libsystem.hpp>

#include "gpio.hpp"

#ifdef ESP_PLATFORM

#include <Arduino.h>
#include <Wire.h>
#include "driver/uart.h"
#include "esp_system.h"

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
    pinMode(12, OUTPUT); // Pin name ?
    pinMode(PIN_HOME_BUTTON, INPUT_PULLUP);
    pinMode(PIN_BATTERY_CHARGING_STATUS, INPUT);

    setVibrator(false);

    /*uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    // Set the source clock in the configuration
    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
        uart_config.source_clk = UART_SCLK_REF_TICK;
    #else
        uart_config.use_ref_tick = true;
    #endif

    // Configure UART0 (you can change to another UART if needed)
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));

    // Set UART pins (using default pins)
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Install UART driver
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0));*/

    #endif
}

void hardware::setScreenPower(bool power)
{
    // TODO : Explain what it does

    #ifdef ESP_PLATFORM

    digitalWrite(12, 0);
    digitalWrite(PIN_SCREEN_POWER, 0);

    delay(500);

    digitalWrite(PIN_SCREEN_POWER, power);

    delay(500);

    digitalWrite(12, 1);

    delay(500);

    digitalWrite(12, 0);

    delay(500);

    digitalWrite(12, 1);

    delay(500);
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

uint8_t hardware::scanI2C(const int sda, const int scl, const bool begin) {
#ifdef ESP_PLATFORM

    if (begin) {
        Wire.begin(sda, scl);
    }

    uint8_t count = 0;

    // Address 0 is broadcast
    for (uint8_t i = 1; i < 128; i++) {
        if (checkI2C(i, sda, scl, false) == SUCCESS) {
            libsystem::log("Found I2C device at address: " + std::to_string(i));
            count++;
        }
    }

    return count;

#else

    return 0;

#endif
}

hardware::I2CResponse hardware::checkI2C(const uint8_t address, const int sda, const int scl, const bool begin) {
#ifdef ESP_PLATFORM
    if (begin) {
        Wire.begin(sda, scl);
    }
    Wire.beginTransmission(address);

    return static_cast<I2CResponse>(Wire.endTransmission());
#else
    return SUCCESS;
#endif
}

bool hardware::isCharging() {
    // Found between 2700 < 3000
    // But tested with good chargers

#ifdef ESP_PLATFORM
    return analogRead(PIN_BATTERY_CHARGING_STATUS) > 100;
#else
    return true;
#endif
}

namespace hardware::input {
    InputFrame lastFrame;
    InputFrame frame;
}

void hardware::input::update() {
    lastFrame = frame;

    frame.homeButtonState = getHomeButton() ? PRESSED : RELEASED;
}

hardware::input::ButtonState hardware::input::getButtonState(const Button button) {
    switch (button) {
        case HOME:
            return getHomeButton() ? PRESSED : RELEASED;
        default:
            throw libsystem::exceptions::InvalidArgument("Unknown button.");
    }
}

bool hardware::input::getButtonDown(const Button button) {
    switch (button) {
        case HOME:
            return lastFrame.homeButtonState == RELEASED && frame.homeButtonState == PRESSED;
        default:
            throw libsystem::exceptions::InvalidArgument("Unknown button.");
    }
}

bool hardware::input::getButtonUp(const Button button) {
    switch (button) {
        case HOME:
            return lastFrame.homeButtonState == PRESSED && frame.homeButtonState == RELEASED;
        default:
            throw libsystem::exceptions::InvalidArgument("Unknown button.");
    }
}
