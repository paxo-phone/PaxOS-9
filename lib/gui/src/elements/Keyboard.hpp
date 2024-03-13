//
// Created by Charles on 13/03/2024.
//

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "../ElementBase.hpp"

namespace gui::elements
{
    class Keyboard final : public ElementBase
    {
    public:
        Keyboard();
        ~Keyboard() override;

        void render() override;
        void onReleased() override;
    };
} // gui::elements

#endif //KEYBOARD_HPP
