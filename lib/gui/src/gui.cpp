//
// Created by Charles on 17/01/2024.
//

#include "gui.hpp"

namespace
{
    std::shared_ptr<graphics::Surface> renderTarget;
}

void gui::setRenderTarget(const graphics::Surface& target)
{
    renderTarget = std::make_shared<graphics::Surface>(target);
}

graphics::Surface* gui::getRenderTarget()
{
    return renderTarget.get();
}
