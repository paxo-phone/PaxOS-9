#include "backtrace.hpp"
#include "gsm.hpp"
#include "path.hpp"
#include "filestream.hpp"

#ifdef ESP_PLATFORM

namespace backtrace_saver {
    

    bool shouldSaveBacktrace()
    {
        return GSM::years != 0;
    }

    void saveBacktrace()
    {
        if (!shouldSaveBacktrace())
            return;

        backtrace_saver::re_restart_debug_t currentData = backtrace_saver::debugGet();
        if (currentData.backtrace[0] != 0)
        {
            std::cout << "Last backtrace: " << std::endl;
            #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
                for (int i = 0; i < CONFIG_RESTART_DEBUG_STACK_DEPTH; i++) {
                    std::cout << std::hex << currentData.backtrace[i] << " ";
                }
                std::cout << std::endl;
            #endif
        }

        storage::Path path = storage::Path("logs/backtrace_" + GSM::getCurrentTimestampNoSpaces() + ".txt");

        storage::FileStream fileStream = storage::FileStream();
        
        fileStream.open(path.str(), storage::Mode::WRITE);
        fileStream.write("heap_total: " + std::to_string(currentData.heap_total) + "\n");
        fileStream.write("heap_free: " + std::to_string(currentData.heap_free) + "\n");
        fileStream.write("heap_free_min: " + std::to_string(currentData.heap_free_min) + "\n");
        fileStream.write("heap_min_time: " + std::to_string(currentData.heap_min_time) + "\n");
        #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
            fileStream.write("backtrace: ");
            for (int i = 0; i < CONFIG_RESTART_DEBUG_STACK_DEPTH; i++) {
                if (currentData.backtrace[i] == 0)
                    break;
                fileStream.write(std::to_string(currentData.backtrace[i]) + " ");
            }
            fileStream.close();
        #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH > 0

        if (backtraceEventId != -1)
            eventHandlerBack.removeEventListener(backtraceEventId);
    }
}

#endif