#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#include "clock.hpp"

namespace hardware
{
    void init();

    void setScreenPower(bool power);
    void setVibrator(bool state);
    bool getHomeButton();

    // https://www.arduino.cc/reference/en/language/functions/communication/wire/endtransmission/
    enum I2CResponse {
        SUCCESS = 0,
        ERROR_DATA_TOO_LONG = 1,
        ERROR_NACK_ADDRESS = 2,
        ERROR_NACK_DATA = 3,
        ERROR_OTHER = 4,
        ERROR_TIMEOUT = 5
    };

    uint8_t scanI2C(int sda = -1, int scl = -1, bool begin = true);
    I2CResponse checkI2C(uint8_t address, int sda = -1, int scl = -1, bool begin = true);

    bool isCharging();
};

#endif