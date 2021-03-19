#pragma once

#include <string.h>

#include <libio/Writer.h>
#include <libutils/String.h>

namespace IO
{

static inline ResultOr<size_t> write(Writer &writer, char value)
{
    return writer.write(&value, sizeof(value));
}

static inline ResultOr<size_t> write(Writer &writer, const char *cstring)
{
    return writer.write(cstring, strlen(cstring));
}

static inline ResultOr<size_t> write(Writer &writer, String string)
{
    return writer.write(string.cstring(), string.length());
}

template <typename T>
static inline void write(Writer &writer, const T &data)
{
    uint8_t *bytes = (uint8_t *)&data;
    writer.write(bytes, sizeof(T));
}

} // namespace IO
