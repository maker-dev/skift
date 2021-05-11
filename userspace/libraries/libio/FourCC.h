#pragma once

#include <libio/Read.h>
#include <libutils/Endian.h>

namespace IO
{

constexpr uint32_t make_tag(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return d + ((uint32_t)c << 8) + ((uint32_t)b << 16) + ((uint32_t)a << 24);
}

const auto read_tag = read<be_uint32_t>;

} // namespace IO
