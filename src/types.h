#ifndef __NESEMUL_TYPES_H__
#define __NESEMUL_TYPES_H__

#include <cstdint>

typedef uint8_t byte_t;
typedef uint16_t address_t;

#define BREAKPOINT __asm__("int $3")
#define DEOPTIMIZE __pragma(optimize("",off))

#endif // __NESEMUL_TYPES_H__
