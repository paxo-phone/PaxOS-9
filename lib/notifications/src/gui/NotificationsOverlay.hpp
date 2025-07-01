#ifndef NOTIFICATIONSOVERLAY_HPP
#define NOTIFICATIONSOVERLAY_HPP

#include "ElementBase.hpp"

namespace gui::elements
{
    class Label;
}

namespace notifications
{
    class NotificationsOverlay final : public gui::ElementBase
    {
        gui::elements::Label* titleLabel_;
        gui::elements::Label* descriptionLabel_;

      public:
        NotificationsOverlay();

        void render() override;
    };
} // namespace notifications

#endif // NOTIFICATIONSOVERLAY_HPP
