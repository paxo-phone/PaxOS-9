//
// Created by Charles on 13/08/2024.
//

#include "overlay.hpp"

#include <libsystem.hpp>
#include <map>
#include <elements/Window.hpp>

#include "graphics.hpp"

namespace gui::overlay {
    std::map<elements::Window*, Overlay*> windowOverlayMap;
    std::vector<ElementBase*> elements;
}

void gui::overlay::init() {

}

void gui::overlay::free() {

}

void gui::overlay::redraw() {
    // Redraw every window
    for (const auto [window, overlay] : windowOverlayMap) {
        overlay->redraw();
    }
}

void gui::overlay::registerWindow(elements::Window* window) {
    auto overlay = new Overlay(window);

    // Inject overlay in the window
    window->addChild(overlay);

    // Keep track of the windows
    windowOverlayMap.insert({
        window,
        overlay
    });
}

void gui::overlay::unregisterWindow(elements::Window* window) {
    windowOverlayMap.erase(window);
}

void gui::overlay::add(ElementBase* element) {
    elements.push_back(element);
}

gui::overlay::Overlay::Overlay(elements::Window* window) {
    m_window = window;

    m_x = 0;
    m_y = 0;
    m_width = graphics::getScreenWidth();
    m_height = graphics::getScreenHeight();
    m_backgroundColor = TFT_RED;

    // Do not allocate surface
    m_isVirtualContainer = true;
}

void gui::overlay::Overlay::redraw() {
    m_isDrawn = false;
    m_isRendered = false;

    m_window->renderAll(true);
}

void gui::overlay::Overlay::render() {
    libsystem::log("Overlay::render()");
}

void gui::overlay::Overlay::preRender() {
    libsystem::log("Overlay::preRender()");

    for (ElementBase* element : elements) {
        // Parent need to be the window
        // Because the overlay does not have any surface allocated to it
        element->m_parent = m_window;

        m_children.push_back(element);
    }
}

void gui::overlay::Overlay::postRender() {
    libsystem::log("Overlay::postRender()");

    for (ElementBase* _ : elements) {
        // TODO : Maybe replace this with a safer call
        m_children.pop_back();
    }
}
