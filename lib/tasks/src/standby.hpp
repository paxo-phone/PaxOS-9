#ifndef STANDBY_HPP
#define STANDBY_HPP

#include <stdint.h>

namespace StandbyMode
{
    void trigger();
    void update();

    void wait();

    void setSleepTime(uint64_t sleepTime);

    bool state();

    void enable();
    void disable();

    void savePower();
    void restorePower();
}

#endif