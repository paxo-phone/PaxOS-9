#ifndef CHECKBOX_HPP
#define CHECKBOX_HPP

#include "../ElementBase.hpp"
#include "Image.hpp"

namespace gui::elements
{
    /**
     * @brief Simply a checkbox widget
     **/
    class Checkbox final : public ElementBase
    {
      public:
        Checkbox(uint16_t x, uint16_t y);
        ~Checkbox() override;

        void render() override;
        void onReleased();
        void setState(bool state);
        bool getState();

      private:
        bool m_state = false;
        Image* m_tickIcon = nullptr;
    };
} // namespace gui::elements

#endif
