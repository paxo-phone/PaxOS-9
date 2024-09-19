//
// Created by Charles on 12/09/2024.
//

#include "GUILibrary.hpp"

#include <app.hpp>
#include <libsystem.hpp>
#include <LuaEnvironment.hpp>
#include <SOL2/sol.hpp>


#include <lua_box.hpp>

namespace paxolua::lib {
    GUILibrary::GUILibrary() {
        m_currentWindow = nullptr;
    }

    void GUILibrary::load(LuaEnvironment *env) {
        sol::table paxo = env->getPaxoNamespace();
        auto gui = paxo["gui"].get_or_create<sol::table>();

        // paxo.gui.widget [abstract]
        gui.new_usertype<LuaWidget>("widget",
            "getX", &LuaWidget::getX,
            "setX", &LuaWidget::setX,
            "getY", &LuaWidget::getY,
            "setY", &LuaWidget::setY,
            "getWidth", &LuaWidget::getWidth,
            "setWidth", &LuaWidget::setWidth,
            "getHeight", &LuaWidget::getHeight,
            "setHeight", &LuaWidget::setHeight,
            "setBackgroundColor", &LuaWidget::setBackgroundColor,
            "setBorderColor",&LuaWidget::setBorderColor,
            "setRadius",&LuaWidget::setRadius,
            "setBorderSize", &LuaWidget::setBorderSize,
            "isEnabled", &LuaWidget::isEnabled,
            "enable", &LuaWidget::enable,
            "disable", &LuaWidget::disable,
            "isTouched", &LuaWidget::isTouched,
            "onClick", &LuaWidget::onClick,
            "getChildAtIndex", &LuaWidget::getChildAtIndex,
            "", sol::no_constructor
        );

        // paxo.gui.window
        gui.new_usertype<LuaWindow>(
            "window", // Using default no-parameter constructor
            sol::base_classes, sol::bases<LuaWidget>()
            // no need for a custom destructor ?
            // std::is_destructible<LuaWindow>() is true
        );

        // paxo.gui.box
        gui.new_usertype<LuaBox>(
            "box", sol::constructors<LuaBox(LuaWidget *, int, int, int, int)>(),
            "setRadius", &LuaBox::setRadius,
            sol::base_classes, sol::bases<LuaWidget>()
        );

        // paxo.gui.label
        gui.new_usertype<LuaLabel>(
            "label", sol::constructors<LuaLabel(LuaWidget *, int, int, int, int)>(),
            "getText", &LuaLabel::getText,
            "setText", &LuaLabel::setText,
            "setFontSize", &LuaLabel::setFontSize,
            "getTextHeight", &LuaLabel::getTextHeight,
            "getTextWidth", &LuaLabel::getTextWidth,
            "setVerticalAlignment", &LuaLabel::setVerticalAlignment,
            "setHorizontalAlignment", &LuaLabel::setHorizontalAlignment,
            "setTextColor", sol::overload(&LuaLabel::setTextColor, &LuaLabel::setTextColorRGB),
            sol::base_classes, sol::bases<LuaWidget>()
        );

        // paxo.gui.canvas
        gui.new_usertype<LuaCanvas>(
            "canvas", sol::initializers([env](LuaCanvas *self, LuaWidget *parent, const int x, const int y, const int width, const int height) {
                // Placement new
                new (self) LuaCanvas(parent, x, y, width, height, env);
            }),
            "setPixel", &LuaCanvas::setPixel,
            "drawRect", &LuaCanvas::drawRect,
            "fillRect", &LuaCanvas::fillRect,
            "drawCircle", &LuaCanvas::drawCircle,
            "fillCircle", &LuaCanvas::fillCircle,
            "drawRoundRect", &LuaCanvas::drawRoundRect,
            "fillRoundRect", &LuaCanvas::fillRoundRect,
            "drawPolygon", &LuaCanvas::drawPolygon,
            "fillPolygon", &LuaCanvas::fillPolygon,
            "drawLine", &LuaCanvas::drawLine,
            "drawText", &LuaCanvas::drawText,
            "drawTextCentered", &LuaCanvas::drawTextCentered,
            "drawTextCenteredInBounds", &LuaCanvas::drawTextCenteredInRect,
            "getTouch", &LuaCanvas::getTouch,
            "onTouch", &LuaCanvas::onTouch,
            sol::base_classes, sol::bases<LuaWidget>()
        );

        // paxo.gui.setWindow()
        gui.set_function("setWindow", [&](LuaWindow *window) {
            m_currentWindow = window;
        });
    }

    void GUILibrary::update(LuaEnvironment *env) {
        if (m_currentWindow != nullptr) {
            m_currentWindow->update();
        }
    }
} // paxolua::lib