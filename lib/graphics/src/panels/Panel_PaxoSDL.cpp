//
// Created by Charles on 14/02/2024.
//

#include "Panel_PaxoSDL.hpp"

void Panel_PaxoSDL::setWindowSize(const uint16_t width, const uint16_t height)
{
    _cfg.panel_width = width;
    _cfg.panel_height = height;

    _width = width;
    _height = height;

    if (monitor.renderer == nullptr)
    {
        // Window not created, wait for the creation using the updated config
        return;
    }

    SDL_Window *window = monitor.window;
    SDL_Renderer *renderer = monitor.renderer;

    // Update window size
    SDL_SetWindowSize(window, width, height);

    // Create a new texture
    SDL_DestroyTexture(monitor.texture);
    monitor.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, width, height);
}

