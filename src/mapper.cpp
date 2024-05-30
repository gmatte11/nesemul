#include "mapper.h"
#include "cartridge.h"

#include "mappers/000.h"
#include "mappers/001.h"
#include "mappers/004.h"

#include <fmt/core.h>
#include <stdexcept>

Mapper* Mapper::create(byte_t ines_code)
{
    switch (ines_code)
    {
    case 0: return new M000();
    case 1: return new M001();
    case 4: return new M004();
    default:
        throw std::runtime_error(fmt::format("Unknown mapper {:03}", ines_code));
    }

    //return nullptr;
}