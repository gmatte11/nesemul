#ifndef __NESEMUL_TYPES_H__
#define __NESEMUL_TYPES_H__

#include <cstdint>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#undef min
#undef max

typedef uint8_t byte_t;
typedef uint16_t address_t;

#define BREAKPOINT DebugBreak()
#define DEOPTIMIZE __pragma(optimize("",off))

#endif // __NESEMUL_TYPES_H__
