#include "Surface.hpp"
#include "color.hpp"
#include "graphics.hpp"

#include <iostream>

int main(int argc, char** argv)
{
    graphics::init();

    auto surface = graphics::Surface(480, 320);
    surface.setPixel(8, 8, graphics::packRGB565(255, 0, 0));

    int i = 0;

    while (graphics::update())
    {
        graphics::clear();

        graphics::setColor(255, 0, 0);
        graphics::fillRect(static_cast<uint16_t>(64 + sin(i) * 128), 16, 32, 32);

        i++;
    }

    graphics::cleanup();

    return 0;
}
