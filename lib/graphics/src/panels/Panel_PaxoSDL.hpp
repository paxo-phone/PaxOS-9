//
// Created by Charles on 14/02/2024.
//

#ifndef PANEL_PAXOSDL_HPP
#define PANEL_PAXOSDL_HPP

#include <LovyanGFX.hpp>

class Panel_PaxoSDL final : public lgfx::Panel_sdl
{
private:
    uint16_t m_width = -1;
    uint16_t m_height = -1;

public:
    Panel_PaxoSDL();

    void setWindowSize(uint16_t width, uint16_t height);
};

#endif //PANEL_PAXOSDL_HPP
