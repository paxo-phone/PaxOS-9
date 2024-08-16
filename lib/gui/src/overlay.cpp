//
// Created by Charles on 13/08/2024.
//

#include "overlay.hpp"

#include <libsystem.hpp>

#include "graphics.hpp"

namespace gui::overlay {
    std::shared_ptr<Overlay> overlay;
}

void gui::overlay::Overlay::render() {

}

void gui::overlay::init() {
    overlay = std::make_shared<Overlay>();

    overlay->m_parent = nullptr;
    overlay->setBackgroundColor(TFT_TRANSPARENT);
}

void gui::overlay::draw() {
    if (overlay == nullptr) {
        throw libsystem::exceptions::RuntimeError("Overlay is null, please check if `gui::overlay::init();` was called.");
    }

    libsystem::log("Overlay draw");

    overlay->renderAll(true);
}

void gui::overlay::add(ElementBase* element) {
    if (overlay == nullptr) {
        throw libsystem::exceptions::RuntimeError("Overlay is null, please check if `gui::overlay::init();` was called.");
    }

    overlay->addChild(element);
}

void gui::overlay::clear() {
    overlay->free();
}
