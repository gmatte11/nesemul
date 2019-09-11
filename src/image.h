#ifndef __NESEMUL_IMAGE_H__
#define __NESEMUL_IMAGE_H__

#include "types.h"

#include <array>

struct Color
{
    byte_t r, g, b, a = 255;
};

template <uint32_t W, uint32_t H>
class Image
{
public:
    static constexpr uint32_t Width = W;
    static constexpr uint32_t Height = H;

    void set(int x, int y, Color color);

    byte_t const* data() const { return buf_.data(); }
    size_t size() const { return buf_.size(); }
    size_t pitch() const { return Width * sizeof(Color); }

public:
    std::array<byte_t, sizeof(Color) * Width * Height> buf_ = {0};
};

template <uint32_t W, uint32_t H>
void Image<W, H>::set(int x, int y, Color color)
{
    size_t pos = (y * pitch()) + x * sizeof(Color);
    std::memcpy(buf_.data() + pos, &color, sizeof(Color));
}

#endif // __NESEMUL_IMAGE_H__
