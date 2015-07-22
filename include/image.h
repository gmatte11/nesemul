#ifndef __NESEMUL_IMAGE_H__
#define __NESEMUL_IMAGE_H__

#include <types.h>

#include <array>

using Image = std::array<byte_t, 3 * 340 * 240>;

#endif // __NESEMUL_IMAGE_H__
