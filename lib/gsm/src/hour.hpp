#ifndef HOUR_HPP
#define HOUR_HPP

#include <stdint.h>
#include <string>

namespace Gsm
{
    namespace Time
    {
        extern uint16_t seconds, minutes, hours, days, months, years;

        void updateHour();

        std::string getCurrentTimestamp();
        std::string getCurrentTimestampNoSpaces();
    }
}

#endif