//
// Created by Charles on 17/01/2024.
//

#ifndef BOX_HPP
#define BOX_HPP

#include "../widget/widget.hpp"

namespace gui::elements
{
    class Box : public Widget
    {
    public:
        Box(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~Box() override;

        void render() override;
    };
} // gui::elements

#endif //BOX_HPP
