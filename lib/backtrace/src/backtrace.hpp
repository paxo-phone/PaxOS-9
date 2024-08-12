#pragma once

#ifdef ESP_PLATFORM

#include <backtrace_saver.hpp>
#include <string>

namespace backtrace_saver {
    extern uint32_t backtraceEventId;

    extern backtrace_saver::re_restart_debug_t currentData; 

    extern bool backtraceSaved;

    bool shouldSaveBacktrace();

    bool isBacktraceEmpty();

    std::string getBacktraceMessage();

    bool saveBacktrace();

    void backtraceMessageGUI();
}

#endif