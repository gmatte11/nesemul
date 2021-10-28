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

inline constexpr byte_t operator "" _byte(unsigned long long v)
{
    return static_cast<byte_t>(v);
}

inline constexpr address_t operator "" _addr(unsigned long long v)
{
    return static_cast<address_t>(v);
}

template <typename T>
struct register_t
{
    using base_t = T;

    base_t get() const { return *reinterpret_cast<base_t const*>(this); }
    void set(base_t value) { (*reinterpret_cast<base_t*>(this)) = value; }

    byte_t get_h() const { static_assert(sizeof(base_t) > 1); return static_cast<byte_t>(get() >> 8); }
    byte_t get_l() const { return static_cast<byte_t>(get() & 0xFF); }

    void set_h(byte_t value) { static_assert(sizeof(base_t) > 1); set((static_cast<base_t>(value) << 8) | (get() & 0xFF)); }
    void set_l(byte_t value) { set((get() & 0xFF00) | static_cast<base_t>(value) & 0xFF); }

    register_t& operator=(base_t value) { set(value); return *this; }
    register_t& operator+=(base_t value) { set(get() + value); return *this; }
    register_t& operator-=(base_t value) { set(get() - value); return *this; }
    register_t& operator&=(base_t value) { set(get() & value); return *this; }
    register_t& operator|=(base_t value) { set(get() | value); return *this; }
    register_t& operator^=(base_t value) { set(get() ^ value); return *this; }
};


#define BREAKPOINT DebugBreak()
#define DEOPTIMIZE __pragma(optimize("",off))

#define ASSERT(X) { if(!(X)) { DebugBreak(); } }
#endif // __NESEMUL_TYPES_H__
