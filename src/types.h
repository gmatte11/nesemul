#ifndef __NESEMUL_TYPES_H__
#define __NESEMUL_TYPES_H__

#include <concepts>
#include <cstdint>
#include <cstring>

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

template <std::unsigned_integral T>
struct register_t
{
    using base_t = T;

    register_t() { set(base_t{}); }
    register_t(base_t value) { set(value); }

    base_t get() const { return *reinterpret_cast<base_t const*>(this); }
    void set(base_t value) { memcpy(this, &value, sizeof(base_t)); }

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

enum class NT_Mirroring : byte_t
{
    None,
    Single,
    Horizontal,
    Vertical,
};

#define NES_BREAKPOINT __debugbreak()
#define NES_DEOPTIMIZE __pragma(optimize("",off))

#define NES_ASSERT(X) { if(!(X)) { __debugbreak(); } }
#endif // __NESEMUL_TYPES_H__
