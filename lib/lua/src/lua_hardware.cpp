#include "lua_hardware.hpp"
#include "hardware.hpp"

#include <iostream>

void LuaHardware::flash(bool flash)
{
    hardware::setLedPower(flash);
}
