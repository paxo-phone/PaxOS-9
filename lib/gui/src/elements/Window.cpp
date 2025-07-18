//
// Created by Gabriel on 12/02/2024.
//

#include "Window.hpp"

#include <Surface.hpp>
#include <algorithm>
#include <cstdio>
#include <graphics.hpp>

namespace gui::elements
{
    Window::Window()
    {
        x_ = 0;
        y_ = 0;
        width_ = graphics::getScreenWidth();
        height_ = graphics::getScreenHeight();
        backgroundColor_ = COLOR_WHITE;

        // windows.push_back(shared_from_this());
    }

    Window::~Window()
    {
        // windows.erase(
        //     std::remove(windows.begin(), windows.end(), shared_from_this()),
        //     windows.end()
        // );
    }

    void Window::render()
    {
        surface_->fillRect(0, 0, width_, height_, backgroundColor_);
    }

    std::vector<std::shared_ptr<ElementBase>> Window::windows;
} // namespace gui::elements
