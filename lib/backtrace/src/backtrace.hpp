#ifndef BACKTRACE_HPP
#define BACKTRACE_HPP

#ifdef ESP_PLATFORM

#include <backtrace_saver.hpp>
#include <string>

namespace backtrace_saver
{
    extern uint32_t backtraceEventId;

    extern backtrace_saver::re_restart_debug_t currentData;

    extern bool backtraceSaved;

    bool shouldSaveBacktrace();

    bool isBacktraceEmpty();

    std::string getBacktraceMessage();

    backtrace_saver::re_restart_debug_t getCurrentBacktrace();

    bool saveBacktrace();

    void backtraceMessageGUI();
} // namespace backtrace_saver

#endif

#endif
