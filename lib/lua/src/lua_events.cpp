#include "lua_events.hpp"

#include "SOL2/sol.hpp"

#include <tasks.hpp>
#include <threads.hpp>
#include <hardware.hpp>
#include <gsm.hpp>
#include "lua_file.hpp"

LuaTime::LuaTime(LuaFile* lua)
{
    this->lua = lua;
    timerFromStart = millis();
}

uint32_t LuaTime::monotonic()
{
    return millis() - timerFromStart;
}

int findIndex(const std::vector<std::string>& vec, const std::string& target) {
    for (std::size_t i = 0; i < vec.size(); ++i) {
        if (vec[i] == target) {
            return static_cast<int>(i); // Convertir en int si nécessaire
        }
    }
    return -1; // Retourner -1 si la chaîne n'est pas trouvée
}

sol::table LuaTime::get(std::string format)
{
    // découper le format en identifiers
    const std::string delimiter = ",";
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type end = format.find(delimiter);

    while (end != std::string::npos) {
        result.push_back(format.substr(start, end - start));
        start = end + 1;
        end = format.find(delimiter, start);
    }

    result.push_back(format.substr(start));

    std::vector<std::string> identifiers = {"s","mi","h","d","mo","y"};

    std::vector<int> date = {GSM::seconds,GSM::minutes,GSM::hours,GSM::days,GSM::months,GSM::years};

    
    // ajouter les valeurs aux index des identifiers

    sol::table array = lua->lua.create_table();

    // Remplir le tableau avec des nombres
    for (int i = 0; i < result.size(); i++) {
        int index = findIndex(identifiers, result[i]);
        if(index!=-1)
            array[i+1] = date[index];
        else
            return lua->lua.create_table();
    }

    // Retourner le tableau Lua
    return array;
}

LuaTimeInterval::LuaTimeInterval(LuaFile* lua, sol::protected_function func, uint32_t interval)
{
    this->lua = lua;
    this->func = func;
    this->interval = interval;
    this->id = lua->eventHandler.setInterval(std::function<void(void)>(std::bind(&LuaTimeInterval::call, this)), interval);
}


uint32_t LuaTimeEvent::addEventListener(LuaFile* lua, sol::protected_function condition, sol::protected_function callback) {
//    this->lua = lua;
    this->condition = condition;
    this->callback = callback;
//    this->interval = interval;
    // this->id = eventHandlerApp.setInterval(std::function<void(void)>(std::bind(&LuaTimeInterval::call, this)),);
    return  0; //eventHandlerApp.addEventListener( (Function *) condition, (Function *) callback);
}



int LuaTimeInterval::getId()
{
    return id;
}

void LuaTimeInterval::call()
{
    try {
        if(func)
        func();
    } catch (const sol::error& e) {
        // Handle Solidity specific errors
        std::cerr << "Sol error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // Handle other standard exceptions
        std::cerr << "Standard error: " << e.what() << std::endl;
    } catch (...) {
        // Handle any other unknown exceptions
        std::cerr << "Unknown error" << std::endl;
    }
}

LuaTimeInterval::~LuaTimeInterval()
{
    lua->eventHandler.removeInterval(id);
}

LuaTimeTimeout::LuaTimeTimeout(LuaFile* lua, sol::protected_function func, uint32_t timeout)
{
    this->lua = lua;
    this->func = func;
    this->timeout = timeout;
    this->id = lua->eventHandler.setTimeout(new Callback<>(std::function<void(void)>(std::bind(&LuaTimeTimeout::call, this))), timeout);
}

int LuaTimeTimeout::getId()
{
    return id;
}

void LuaTimeTimeout::call()
{
    if(func)
    {
        sol::protected_function_result result = func();

        // Check for errors
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Lua Error: " << err.what() << std::endl;
        }
    }

    done = true;
}

LuaTimeTimeout::~LuaTimeTimeout()
{
    if(!done)
        lua->eventHandler.removeTimeout(id);
}

void LuaTime::update()
{
    running = true;
    lua->eventHandler.update();
    
    for (int it = 0; it < timeouts.size(); it++)
    {
        if (timeouts[it]->done)
        {
            delete timeouts[it];
            timeouts.erase(timeouts.begin() + it);
            it = 0; // reset the loop
        }
    }
    running = false;
}

int LuaTime::setInterval(sol::protected_function func, uint32_t interval)
{
    LuaTimeInterval* n = new LuaTimeInterval(lua, func, interval);
    intervals.push_back(n);
    return n->getId();
}

int LuaTime::setTimeout(sol::protected_function func, uint32_t interval)
{
    LuaTimeTimeout* n = new LuaTimeTimeout(lua, func, interval);
    timeouts.push_back(n);
    return n->getId();
}

void LuaTime::removeInterval(int id)
{
    for (int it = 0; it < intervals.size(); it++)
    {
        if (intervals[it]->getId() == id)
        {
            delete intervals[it];
            intervals.erase(intervals.begin() + it);

            break;
        }
    }
}

void LuaTime::removeTimeout(int id)
{
    for (int it = 0; it < timeouts.size(); it++)
    {
        if (timeouts[it]->getId() == id)
        {
            delete timeouts[it];
            timeouts.erase(timeouts.begin() + it);

            break;
        }
    }
}

LuaTime::~LuaTime()
{
    for (int i = 0; i < intervals.size(); i++)
        delete intervals[i];
    for (int i = 0; i < timeouts.size(); i++)
        delete timeouts[i];
}
