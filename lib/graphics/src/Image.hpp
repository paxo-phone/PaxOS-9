//
// Created by Charles on 20/01/2024.
//

#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <cstdint>
#include <memory>
#include <string>

namespace graphics
{
    enum ImageType
    {
        BMP,
        PNG,
        JPG
    };

    class Image
    {
    private:
        ImageType type;

        uint32_t width;
        uint32_t height;

        uint32_t size;
        std::shared_ptr<uint8_t[]> data;

    public:
        explicit Image(const std::string& filename);
        ~Image();

        [[nodiscard]] uint32_t getWidth() const;
        [[nodiscard]] uint32_t getHeight() const;

        [[nodiscard]] uint32_t getSize() const;
        [[nodiscard]] uint8_t * getData() const;
    };
} // graphics

#endif //IMAGE_HPP
