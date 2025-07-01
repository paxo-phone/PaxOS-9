#ifndef NOTIFICATIONS_HPP
#define NOTIFICATIONS_HPP

namespace Notifications
{
    constexpr unsigned long long OVERLAY_DURATION = 2000;

    void init();
    void teardown();

    void update();

    bool isOverlayVisible();
} // namespace Notifications

#endif // NOTIFICATIONS_HPP
