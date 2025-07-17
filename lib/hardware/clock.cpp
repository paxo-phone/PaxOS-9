#include "clock.hpp"

#if defined(__linux__) || defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)

#include <chrono>
#include <cstdint>

uint64_t os_micros(void)
{
    using namespace std::chrono;
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count() -
           1677588060000000;
}

uint64_t os_millis(void)
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count() -
           1677588060000;
}

#else

unsigned long sleepStartTimeMillis;
long long sleepTimeCorrectionOffset = 0;

unsigned long os_millis()
{
    return millis() + sleepTimeCorrectionOffset;
}

#endif
