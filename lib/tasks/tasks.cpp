#include "tasks.hpp"

uint64_t milliseconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

EventHandler::~EventHandler()
{
    while (events.size())
    {
        delete events[0];
        events.erase(events.begin());
    }
    while (timeouts.size())
    {
        delete timeouts[0];
        timeouts.erase(timeouts.begin());
    }
    while (intervals.size())
    {
        delete intervals[0];
        intervals.erase(intervals.begin());
    }
}

void EventHandler::update()
{
    // Handle events
    for (auto& event : events) {
        if (event->condition->call()) {
            event->callback->call();
        }
    }

    // Handle timeouts
    auto now = milliseconds();
    for (auto it = timeouts.begin(); it != timeouts.end();) {
        if (now  >= (*it)->timeout) {
            (*it)->callback->call();
            delete (*it);
            it = timeouts.erase(it);
        } else {
            ++it;
        }
    }

    // Handle intervals
    for (auto& interval : intervals) {
        if (now >= interval->lastTrigger + interval->interval) {
            interval->callback->call();
            interval->lastTrigger = now;
        }
    }
}

uint32_t EventHandler::addEventListener(Function* condition, Function* callback) {
    uint32_t id = findAvailableId();
    events.push_back(new Event(condition, callback, id));
    return id;
}

void EventHandler::removeEventListener(uint32_t id) {
    events.erase(std::remove_if(events.begin(), events.end(),
                                [id](Event* event) {
                                    return event->id == id;
                                }),
                    events.end());
}

uint32_t EventHandler::setTimeout(Function* callback, uint64_t timeout) {
    uint32_t id = findAvailableId();
    timeouts.push_back(new Timeout(callback, milliseconds() + timeout, id));
    return id;
}

void EventHandler::removeTimeout(uint32_t id) {
    timeouts.erase(std::remove_if(timeouts.begin(), timeouts.end(),
                                [id](Timeout* timeout) {
                                    return timeout->id == id;
                                }),
                    timeouts.end());
}

uint32_t EventHandler::setInterval(Function* callback, uint64_t interval) {
    uint32_t id = findAvailableId();
    intervals.push_back(new Interval(callback, interval, id));
    return id;
}

void EventHandler::removeInterval(uint32_t id) {
    intervals.erase(std::remove_if(intervals.begin(), intervals.end(),
                                    [id](Interval* interval) {
                                        return interval->id == id;
                                    }),
                    intervals.end());
}

uint32_t EventHandler::findAvailableId() {
    uint32_t nextId = 0;
    
    bool found = false;

    while (!found)
    {
        found = true;
        for (auto& event : events)
        {
            if(event->id == nextId)
                found = false;
        }
        for (auto& timeout : timeouts)
        {
            if(timeout->id == nextId)
                found = false;
        }
        for (auto& interval : intervals)
        {
            if(interval->id == nextId)
                found = false;
        }
    }

    return nextId;
}