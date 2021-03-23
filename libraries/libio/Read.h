#pragma once

#include <libio/Copy.h>
#include <libio/Reader.h>
#include <libtest/AssertEqual.h>
#include <libutils/Vector.h>

namespace IO
{

template <typename T>
ResultOr<size_t> read_vector(Reader &reader, Vector<T> &vector)
{
    size_t read = 0;

    T object;
    size_t last_read = TRY(reader.read(&object, sizeof(T)));
    while (last_read > 0)
    {
        vector.push_back(move(object));
        read += last_read;

        last_read = TRY(reader.read(&object, sizeof(T)));
    }

    return read;
}

inline ResultOr<String> read_string(Reader &reader, size_t len)
{
    IO::MemoryWriter memory;
    TRY(IO::copy(reader, memory, len));
    return String{memory.string()};
}

// Peek & Get functons
template <typename T>
inline ResultOr<T> peek(SeekableReader auto &reader)
{
    auto result = TRY(read<T>(reader));
    reader.seek(IO::SeekFrom::current(-sizeof(T)));
    return result;
}

template <typename T>
inline ResultOr<T> read(Reader &reader)
{
    T result;
    size_t read = TRY(reader.read(&result, sizeof(T)));
    assert_equal(read, sizeof(T));
    return result;
}

} // namespace IO
