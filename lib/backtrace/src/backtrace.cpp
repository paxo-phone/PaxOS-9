#include "backtrace.hpp"

#ifdef ESP_PLATFORM

#include <filestream.hpp>
#include <gsm2.hpp>
#include <gui.hpp>
#include <iostream>
#include <path.hpp>
#include <sstream>
#include <string>
#include <threads.hpp>

namespace backtrace_saver
{
    uint32_t backtraceEventId = -1;

    backtrace_saver::re_restart_debug_t currentData = backtrace_saver::debugGet();

    bool backtraceSaved = false;

    bool shouldSaveBacktrace()
    {
        return !backtraceSaved && !isBacktraceEmpty();
    }

    bool isBacktraceEmpty()
    {
        return currentData.backtrace[0].first == 0;
    }

    std::string getBacktraceMessage()
    {
        std::stringstream backtraceMessageStream;
        backtraceMessageStream << "Crash report:\n";
        backtraceMessageStream << "heap_total: " << std::to_string(currentData.heap_total) << "\n";
        backtraceMessageStream << "heap_free: " << std::to_string(currentData.heap_free) << "\n";
        backtraceMessageStream << "heap_free_min: " << std::to_string(currentData.heap_free_min)
                               << "\n";
        backtraceMessageStream << "heap_min_time: " << std::to_string(currentData.heap_min_time)
                               << "\n";

#if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
        backtraceMessageStream << "backtrace: ";
        for (int i = 0; i < CONFIG_RESTART_DEBUG_STACK_DEPTH; i++)
        {
            if (currentData.backtrace[i].first == 0)
                break;
            backtraceMessageStream << "0x" << std::hex << currentData.backtrace[i].first << ":0x"
                                   << std::hex << currentData.backtrace[i].second << " ";
        }
        backtraceMessageStream << "\n";
#endif // CONFIG_RESTART_DEBUG_STACK_DEPTH > 0

        backtraceMessageStream << "\n";

        return backtraceMessageStream.str();
    }

    re_restart_debug_t getCurrentBacktrace()
    {
        re_restart_debug_t oldData = _debug_info;
        debugUpdate();

        re_restart_debug_t returnData = _debug_info;

        _debug_info = oldData;

        return returnData;
    }

    bool saveBacktrace()
    {
        if (!shouldSaveBacktrace())
            return false;
        if (!storage::Path("logs").exists() && !storage::Path("logs").newdir())
        {
            std::cout << "Couldn't create logs/ directory to save backtrace" << std::endl;
            return false;
        }

        storage::Path path = storage::Path("logs/backtrace.txt");

        if (!path.exists() && !path.newfile())
        {
            std::cout << "Couldn't create backtrace file" << std::endl;
            return false;
        }

        storage::FileStream fileStream = storage::FileStream();

        fileStream.open(path.str(), storage::Mode::APPEND);
        fileStream.write(getBacktraceMessage());
        fileStream.close();

        std::cout << "Backtrace saved to " << path.str() << std::endl;

        if (backtraceEventId != -1)
            eventHandlerBack.removeEventListener(backtraceEventId);

        return true;
    }

    void backtraceMessageGUI()
    {
        Window win;

        const auto label = new Label(0, 0, 320, 400);

        label->setText(
            "This Paxo crashed and the backtrace was saved.\nPlease report this issue to the "
            "developers.\n\n" +
            getBacktraceMessage()
        );
        win.addChild(label);

        auto* printBacktraceButton = new gui::elements::Button(35, 370, 250, 38);
        printBacktraceButton->setText("Print backtrace to monitor");
        win.addChild(printBacktraceButton);

        auto* quitButton = new gui::elements::Button(35, 420, 250, 38);
        quitButton->setText("Quitter");
        win.addChild(quitButton);

        // Wait for action before closing GUI
        while (!hardware::getHomeButton()) // TODO: asynchronize this
        {
            win.updateAll();
            if (quitButton->isTouched())
                return;
            if (printBacktraceButton->isTouched())
                std::cout << getBacktraceMessage() << std::endl;
        }

        return;
    }
} // namespace backtrace_saver

#endif
