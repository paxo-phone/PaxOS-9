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

        static std::vector<std::shared_ptr<gui::ElementBase>> windows;

        void render() override;
    };
} // namespace gui::elements

#endif // WINDOW_HPP
