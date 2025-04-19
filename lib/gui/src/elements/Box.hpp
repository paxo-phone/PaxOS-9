//
// Created by Charles on 17/01/2024.
//

#ifndef BOX_HPP
#define BOX_HPP


#include "../ElementBase.hpp"

namespace gui::elements
{
    /**
     * @brief A simple box that can be used as a background, border, etc.
     */
    class Box final : public ElementBase
    {
    public:
        Box(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~Box() override;

        void render() override;
    };
} // gui::elements

#endif //BOX_HPP
