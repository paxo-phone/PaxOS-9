#ifndef SWITCH_HPP
#define SWITCH_HPP

#include "../ElementBase.hpp"

namespace gui::elements
{
    class Switch final : public ElementBase
    {
      public:
        Switch(uint16_t x, uint16_t y);
        ~Switch();

        void render() override;
        void onReleased();
        void setState(bool state);
        bool getState();

      private:
        bool m_state = false;
    };
} // namespace gui::elements

#endif
