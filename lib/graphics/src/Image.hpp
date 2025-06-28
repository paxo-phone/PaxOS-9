//
// Created by Charles on 20/01/2024.
//

#ifndef SIMAGE_HPP
#define SIMAGE_HPP

#include <cstdint>
#include <memory>
#include <path.hpp>
#include <string>

namespace graphics
{
    enum ImageType
    {
        BMP,
        PNG,
        JPG
    };

    class SImage
    {
      private:
        ImageType m_type;
        storage::Path m_path;

        uint16_t m_width;
        uint16_t m_height;

        uint32_t m_size;
        std::shared_ptr<uint8_t[]> m_data;

      public:
        explicit SImage(storage::Path& filename);
        ~SImage();

        [[nodiscard]] ImageType getType() const;

        [[nodiscard]] uint16_t getWidth() const;
        [[nodiscard]] uint16_t getHeight() const;

        [[nodiscard]] uint32_t getSize() const;
        [[nodiscard]] uint8_t* getData() const;

        [[nodiscard]] storage::Path getPath() const;
    };
} // namespace graphics

#endif // SIMAGE_HPP
