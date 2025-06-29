#ifndef FILTER_HPP
#define FILTER_HPP

#include "../ElementBase.hpp"

#include <filestream.hpp>

namespace gui::elements
{
    /**
     * @deprecated Not compatible with device.
     */
    class Filter final : public ElementBase
    {
      public:
        typedef std::pair<int16_t, int16_t> point_t;

        Filter(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        ~Filter() override;

        void render() override;

        void apply() const;

      private:
        std::shared_ptr<graphics::Surface> m_screenSurface;
    };
} // namespace gui::elements

#endif // FILTER_HPP
