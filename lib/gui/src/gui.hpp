#ifndef GUI_HPP
#define GUI_HPP

#include <Surface.hpp>

namespace gui
{
    void setRenderTarget(const graphics::Surface& target);
    graphics::Surface * getRenderTarget();
}

#endif