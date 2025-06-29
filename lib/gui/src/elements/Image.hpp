#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "../ElementBase.hpp"

#include <path.hpp>

namespace gui::elements
{
    /**
     * @brief Widget image: can load an image from a path with a background or not.
     *Support png, jpeg and bmp
     **/
    class Image final : public ElementBase
    {
      public:
        Image(
            storage::Path path, uint16_t x, uint16_t y, uint16_t width, uint16_t height,
            color_t backgroundColor = COLOR_WHITE
        );
        ~Image() override;

        void render() override;

        void load(color_t background = COLOR_WHITE);
        void setTransparentColor(color_t color);

      private:
        storage::Path m_path;
        color_t m_backgroundColor;
    };
} // namespace gui::elements

#endif // BOX_HPP
