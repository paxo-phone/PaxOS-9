#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#include "clock.hpp"

namespace hardware
{
    void init();

    void setScreenPower(bool power);
    void setVibrator(bool state);
    bool getHomeButton();
};

#endif