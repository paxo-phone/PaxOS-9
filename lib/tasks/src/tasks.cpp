#include "tasks.hpp"

#include <algorithm>
#include <clock.hpp>
#include <iostream>

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

    intervals.clear();
}

void EventHandler::update(std::function<bool()> forced_exit)
{
    // Handle events
    for (auto& event : events)
        if (event->condition->call())
            event->callback->call();

    // Handle timeouts
    auto now = os_millis();
    for (auto it = timeouts.begin(); it != timeouts.end();)
    {
        if (now >= (*it)->timeout)
        {
            auto* timeout = *it;
            it = timeouts.erase(it);
            timeout->callback->call();
            delete timeout;
        }
        else
        {
            ++it;
        }

        if (forced_exit())
            return;
    }

    // Handle intervals
    try
    {
        for (int i = 0; i < intervals.size(); i++)
        {
            if (intervals.size() > i)
            {
                if (intervals[i].callback)
                {
                    if (now >= intervals[i].lastTrigger + intervals[i].interval)
                    {
                        intervals[i].callback();
                        intervals[i].lastTrigger = now;
                    }
                }
                else
                {
                    intervals.erase(intervals.begin() + i);
                    i--;
                }
            }

            if (forced_exit())
                return;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

uint32_t EventHandler::addEventListener(Function* condition, Function* callback)
{
    uint32_t id = findAvailableId();
    events.push_back(new Event(condition, callback, id));
    return id;
}

void EventHandler::removeEventListener(uint32_t id)
{
    events.erase(
        std::remove_if(
            events.begin(),
            events.end(),
            [id](Event* event)
            {
                return event->id == id;
            }
        ),
        events.end()
    );
}

uint32_t EventHandler::setTimeout(Function* callback, uint64_t timeout)
{
    uint32_t id = findAvailableId();
    timeouts.push_back(new Timeout(callback, os_millis() + timeout, id));
    return id;
}

void EventHandler::removeTimeout(uint32_t id)
{
    timeouts.erase(
        std::remove_if(
            timeouts.begin(),
            timeouts.end(),
            [id](Timeout* timeout)
            {
                return timeout->id == id;
            }
        ),
        timeouts.end()
    );
}

uint32_t EventHandler::setInterval(std::function<void()> callback, uint64_t interval)
{
    uint32_t id = findAvailableId();
    intervals.push_back(Interval(callback, interval, id));
    return id;
}

void EventHandler::removeInterval(uint32_t id)
{
    intervals.erase(
        std::remove_if(
            intervals.begin(),
            intervals.end(),
            [id](Interval& interval)
            {
                return interval.id == id;
            }
        ),
        intervals.end()
    );
}

uint32_t EventHandler::findAvailableId()
{
    uint32_t nextId = 0;

    bool found = false;

    while (!found)
    {
        found = true;
        for (auto& event : events)
            if (event->id == nextId)
                found = false;
        for (auto& timeout : timeouts)
            if (timeout->id == nextId)
                found = false;
        for (auto& interval : intervals)
            if (interval.id == nextId)
                found = false;

        nextId++;
    }

    return nextId - 1;
}
