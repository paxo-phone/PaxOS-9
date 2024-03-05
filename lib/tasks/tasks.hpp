#ifndef TASKS_HPP
#define TASKS_HPP

#include <stdlib.h>
#include <functional>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <tuple>
#include "invoke.hpp"

uint64_t milliseconds();

class Function
{
public:
    virtual bool call() = 0;
};

template<typename... ArgsCo>
class Condition : public Function
{
public:
    std::function<bool(ArgsCo...)> condition;
    std::tuple<ArgsCo...> argsCo;

    Condition(std::function<bool(ArgsCo...)> condition, ArgsCo... args)
        : condition(condition), argsCo(std::make_tuple(args...)) {}
    
    ~Condition() 
    {
    }

    bool call() override
    {
        return invoke_hpp::apply(condition, argsCo);
    }
};

template<typename... ArgsCa>
class Callback : public Function
{
public:
    std::function<void(ArgsCa...)> callback;
    std::tuple<ArgsCa...> argsCa;

    Callback(std::function<void(ArgsCa...)> callback, ArgsCa... args)
        : callback(callback), argsCa(std::make_tuple(args...)) {}

    ~Callback()
    {
    }

    bool call() override
    {
        invoke_hpp::apply(callback, argsCa);
        return true;
    }
};


class Event {
public:
    Function* condition;
    Function* callback;
    uint64_t id;
    
    Event(Function* co, Function* ca, uint64_t id)
    {
        this->condition = co;
        this->callback = ca;
        this->id = id;
    }

    ~Event()
    {
        delete condition;
        delete callback;
    }
};

class Interval {
public:
    Function* callback;
    uint64_t interval;
    uint64_t lastTrigger;
    uint64_t id;

    Interval(Function* ca, uint64_t interval, uint64_t id)
    {
        this->callback = ca;
        this->interval = interval;
        this->lastTrigger = milliseconds();
        this->id = id;
    }

    ~Interval()
    {
        delete callback;
    }
};

class Timeout {
public:
    Function* callback;
    uint64_t timeout;
    uint64_t id;

    Timeout(Function* ca, uint64_t timeout, uint64_t id)
    {
        this->callback = ca;
        this->timeout = timeout;
        this->id = id;
    }

    ~Timeout()
    {
        delete callback;
    }
};


class EventHandler
{
    public:
    std::vector<Event*> events;
    std::vector<Timeout*> timeouts;
    std::vector<Interval*> intervals;

    ~EventHandler();

    void update();

    uint32_t addEventListener(Function* condition, Function* callback);
    void removeEventListener(uint32_t id);
    uint32_t setTimeout(Function* callback, uint64_t timeout);
    void removeTimeout(uint32_t id);
    uint32_t setInterval(Function* callback, uint64_t interval);
    void removeInterval(uint32_t id);

private:    
    uint32_t findAvailableId();
};

#endif