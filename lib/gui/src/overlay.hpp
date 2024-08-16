//
// Created by Charles on 13/08/2024.
//

#ifndef GUI_OVERLAY_HPP
#define GUI_OVERLAY_HPP

#include <ElementBase.hpp>

namespace gui::overlay {
    void init();
    void draw();

    void add(ElementBase* element);
    void clear();

    class Overlay final : public ElementBase {
    public:
        Overlay() = default;
        ~Overlay() override = default;

        void render() override;
    };
}

#endif //GUI_OVERLAY_HPP
