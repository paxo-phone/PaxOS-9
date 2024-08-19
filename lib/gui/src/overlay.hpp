//
// Created by Charles on 13/08/2024.
//

#ifndef GUI_OVERLAY_HPP
#define GUI_OVERLAY_HPP

#include <ElementBase.hpp>

namespace gui::elements {
    class Window;
}

namespace gui::overlay {
    /**
    * Initialize the overlay.
    */
    void init();

    /**
    * Free the memory used by the overlay.
    */
    void free();

    /**
     * Redraw the active window
     */
    void redraw();

    /**
     * Add an GUI element to the overlay.
     * @param element The element to add.
     */
    void add(ElementBase* element);

    /**
     * Register a window to add the overlay render hook into it.
     * @param window The window to add the overlay to.
     */
    void registerWindow(elements::Window* window);

    /**
     * Remove the overlay hook from the window.
     * @param window The window to unregister.
     */
    void unregisterWindow(elements::Window* window);

    class Overlay final : public ElementBase {
    public:
        explicit Overlay(elements::Window* window);

        ~Overlay() override = default;

        void redraw();

        void render() override;

        void preRender() override;
        void postRender() override;

    private:
        elements::Window* m_window;
    };
}

#endif //GUI_OVERLAY_HPP
