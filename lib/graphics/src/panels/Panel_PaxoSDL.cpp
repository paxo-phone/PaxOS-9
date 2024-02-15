//
// Created by Charles on 14/02/2024.
//

#include "Panel_PaxoSDL.hpp"

Panel_PaxoSDL::Panel_PaxoSDL() : Panel_sdl()
{
    m_width = _cfg.panel_width;
    m_height = _cfg.panel_height;
}

void Panel_PaxoSDL::setWindowSize(const uint16_t width, const uint16_t height)
{
    // This shit is broken, don't uncomment it
    // _cfg.panel_width = width;
    // _cfg.panel_height = height;
    // _width = width;
    // _height = height;

    printf("setWindowSize(%d, %d)\n", width, height);

    m_width = width;
    m_height = height;

    SDL_Window *window = monitor.window;
    SDL_Renderer *renderer = monitor.renderer;

    // Update window size
    setWindowTitle("Hello World :)");
    SDL_SetWindowSize(window, m_width * monitor.scaling_x, m_height * monitor.scaling_y);

    // Create a new texture
    SDL_DestroyTexture(monitor.texture);
    monitor.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, m_width, m_height);
}

