#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "../ElementBase.hpp"
#include <path.hpp>

namespace gui::elements
{
    class Image final : public ElementBase
    {
    public:
        Image(storage::Path path, uint16_t x, uint16_t y, uint16_t width, uint16_t height, color_t backgroundColor = COLOR_WHITE);
        ~Image() override;

        void render() override;

        void load(color_t background = COLOR_WHITE);
        void setTransparentColor(color_t);

    private:
        storage::Path m_path;
        color_t m_backgroundColor;
    };
} // gui::elements

#endif //BOX_HPP
