//
// Created by Charles on 17/01/2024.
//

#ifndef BOX_HPP
#define BOX_HPP

#include "../ElementBase.hpp"

namespace gui::elements
{
    class Box final : public ElementBase
    {
    private:
        uint8_t r;
        uint8_t g;
        uint8_t b;

    public:
        Box(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~Box() override;

        void render() override;

        void getColor(uint8_t *r, uint8_t *g, uint8_t *b) const;
        void setColor(uint8_t r, uint8_t g, uint8_t b);
    };
} // gui::elements

#endif //BOX_HPP
