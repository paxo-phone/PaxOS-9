#include "Notifications.hpp"

#include "app.hpp"
#include "graphics.hpp"
#include "gsm2.hpp"
#include "gui/NotificationsOverlay.hpp"
#include "libsystem.hpp"

namespace
{
    Window* targetWindow = nullptr;
    notifications::NotificationsOverlay* targetOverlay = nullptr;
    unsigned long long lastNotificationTime = 0;
} // namespace

static bool windowHasNotificationsOverlay(Window* w)
{
    if (w == nullptr)
        return false;

    const std::any notificationsMetadata = w->getMetadata("notifications");

    return notificationsMetadata.has_value();
}

static notifications::NotificationsOverlay* windowGetOverlay(Window* w)
{
    if (w == nullptr)
        return nullptr;
    if (!windowHasNotificationsOverlay(w))
        return nullptr;
    // ReSharper disable once CppDFANullDereference
    if (const std::any notificationsMetadata = w->getMetadata("notifications");
        notificationsMetadata.has_value())
        return std::any_cast<notifications::NotificationsOverlay*>(notificationsMetadata);
    return nullptr;
}

static notifications::NotificationsOverlay* injectOverlay(Window* w)
{
    if (w == nullptr)
        return nullptr;
    if (windowHasNotificationsOverlay(w))
    {
        notifications::NotificationsOverlay* overlay = windowGetOverlay(w);

        overlay->setEnabled(true);
        libsystem::log("[Notifications] Enabled overlay of window.");
        return overlay;
    }

    auto* overlay = new notifications::NotificationsOverlay();

    w->addChild(overlay);
    w->setMetadata("notifications", overlay);
    libsystem::log("[Notifications] Injected overlay into window.");
    return overlay;
}

void Notifications::init()
{
    GuiManager& guiManager = GuiManager::getInstance();

    guiManager.addWindowChangedCallback(
        [](Window* w)
        {
            targetWindow = w;
            targetOverlay = injectOverlay(targetWindow);
        }
    );

    lastNotificationTime = libsystem::getMillis();
}

void Notifications::teardown()
{
    // TODO: Remove the overlay drawing callback.
}

void Notifications::update()
{
    if (targetOverlay == nullptr)
        return;
    targetOverlay->setEnabled(isOverlayVisible());
}

bool Notifications::isOverlayVisible()
{
    return libsystem::getMillis() <= lastNotificationTime + OVERLAY_DURATION;
}
