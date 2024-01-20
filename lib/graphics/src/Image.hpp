//
// Created by Charles on 20/01/2024.
//

#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <cstdint>
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
        uint8_t *data;

        uint8_t *rawData;

    public:
        explicit Image(ImageType type);
        ~Image();

        [[nodiscard]] uint32_t getWidth() const;
        [[nodiscard]] uint32_t getHeight() const;

        [[nodiscard]] uint32_t getSize() const;

        [[nodiscard]] uint8_t * getData8() const; // get image 8 bits
        [[nodiscard]] uint16_t * getData16() const; // get image 16 bits
        [[nodiscard]] uint32_t * getData32() const; // get image 32 bits

        [[nodiscard]] uint8_t * getRawData() const;

        void loadBMP(const std::string& filename);
    };
} // graphics

#endif //IMAGE_HPP
