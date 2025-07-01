//
// Created by charl on 01/07/2025.
//

#include "NotificationsOverlay.hpp"

#include "Notifications.hpp"
#include "elements/Label.hpp"
#include "graphics.hpp"

namespace notifications
{
    NotificationsOverlay::NotificationsOverlay()
    {
        m_x = 10;
        m_y = 10;
        m_width = graphics::getScreenWidth() - 20;
        m_height = 100;

        titleLabel_ = new gui::elements::Label(10, 10, m_width - 20, 20);
        descriptionLabel_ = new gui::elements::Label(10, 30, m_width - 20, m_height - 40);

        titleLabel_->setText("Notification Title");
        titleLabel_->setTextColor(COLOR_BLACK);
        titleLabel_->setFontSize(20);

        descriptionLabel_->setText(
            "This is a test notification description. It can be longer than the title."
        );
        descriptionLabel_->setTextColor(COLOR_BLACK);
        descriptionLabel_->setFontSize(16);

        addChild(titleLabel_);
        addChild(descriptionLabel_);
        setEnabled(Notifications::isOverlayVisible());
    }

    void NotificationsOverlay::render()
    {
        m_surface->setTransparency(true);
        m_surface->setTransparentColor(TFT_TRANSPARENT);
        m_surface->fillRect(0, 0, m_width, m_height, TFT_TRANSPARENT);
        m_surface
            ->fillRoundRectWithBorder(0, 0, m_width, m_height, 15, 3, COLOR_WHITE, COLOR_BLACK);
    }
} // namespace notifications
