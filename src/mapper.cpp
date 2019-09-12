#include "mapper.h"

#include "mappers/000.h"

Mapper* Mapper::create(byte_t ines_code)
{
    switch (ines_code)
    {
    case 0: return new M000;
    }

    return nullptr;
}