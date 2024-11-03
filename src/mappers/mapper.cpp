#include "mappers/mapper.h"
#include "cartridge.h"

#include "mappers/000.h"
#include "mappers/001.h"
#include "mappers/004.h"

#include <fmt/core.h>
#include <stdexcept>

Mapper* Mapper::create(byte_t ines_code, Cartridge& cart)
{
    Mapper* mapper = nullptr;

    switch (ines_code)
    {
    case 0: mapper = new M000(cart); break;
    case 1: mapper = new M001(cart); break;
    case 4: mapper = new M004(cart); break;
    default:
        throw std::runtime_error(fmt::format("Unknown mapper {:03}", ines_code));
    }

    return mapper;
}