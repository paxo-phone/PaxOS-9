//
// Created by Gabriel on 12/02/2024.
//

#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "../ElementBase.hpp"

namespace gui::elements
{
    class Window final : public ElementBase
    {
    private:

    public:
        Window();
        ~Window() override;

        static std::vector<Window*> windows;

        void render() override;
    };
} // gui::elements

#endif //WINDOW_HPP
