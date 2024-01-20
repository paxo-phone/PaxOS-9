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
        BITMAP,
        PNG,
        JPG
    };

    class Image
    {
    private:
        ImageType type;

    public:
        explicit Image(ImageType type);
        ~Image();

        [[nodiscard]] uint8_t getData8() const; // get image 8 bits
        [[nodiscard]] uint16_t getData16() const; // get image 16 bits
        [[nodiscard]] uint32_t getData32() const; // get image 32 bits

        void loadBitmap(const std::string& filename);
    };
} // graphics

#endif //IMAGE_HPP
