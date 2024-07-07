#pragma once

#ifdef ESP_PLATFORM

#include <backtrace_saver.hpp>

namespace backtrace_saver {
    inline uint32_t backtraceEventId = -1;

    bool shouldSaveBacktrace();

    void saveBacktrace();
}

#endif