#pragma once

#include <libio/Copy.h>
#include <libio/Reader.h>
#include <libutils/Array.h>
#include <libutils/Assert.h>
#include <libutils/Vector.h>

namespace IO
{

template <typename T>
inline ResultOr<T> read(Reader &reader)
{
    T result;
    size_t read = TRY(reader.read(&result, sizeof(T)));
    Assert::equal(read, sizeof(T));
    return result;
}

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

template <typename T>
ResultOr<Vector<T>> read_vector(Reader &reader, size_t n)
{
    Vector<T> result;
    result.resize(n);

    for (size_t i = 0; i < n; i++)
    {
        result[i] = TRY(IO::read<T>(reader));
    }

    return result;
}

template <typename T, size_t N>
ResultOr<Array<T, N>> read_array(Reader &reader)
{
    Array<T, N> result;

    for (size_t i = 0; i < N; i++)
    {
        result[i] = TRY(IO::read<T>(reader));
    }

    return result;
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

} // namespace IO
