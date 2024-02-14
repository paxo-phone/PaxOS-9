//
// Created by Charles on 14/02/2024.
//

#ifndef PANEL_PAXOSDL_HPP
#define PANEL_PAXOSDL_HPP

#include <LovyanGFX.hpp>

class Panel_PaxoSDL final : public lgfx::Panel_sdl
{
public:
    void setWindowSize(uint16_t width, uint16_t height);
};

#endif //PANEL_PAXOSDL_HPP
