#ifndef RADIO_HPP
#define RADIO_HPP

#include "../ElementBase.hpp"
#include "Image.hpp"

namespace gui::elements
{
    class Radio final : public ElementBase
    {
      public:
        Radio(uint16_t x, uint16_t y);
        ~Radio() override;

        void render() override;
        void onReleased();
        void setState(bool state);
        bool getState();

      private:
        bool m_state = false;
    };
} // namespace gui::elements

#endif
